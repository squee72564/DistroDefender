#include <geo/locator.hpp>

#include <cinttypes>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <shared_mutex>
#include <sstream>

#include <fmt/format.h>

#include <geo/dbEntry.hpp>
#include <geo/manager.hpp>

namespace
{

static constexpr std::string_view TRANSLATE_ERROR{"Error translating IP address"};
static constexpr std::string_view LIBMD_ERROR{"Error from libmaxminddb:"};

std::vector<const char *> getPathCStrVec(const DotPath& path)
{
    std::vector<const char*> pathCStrVec{};
    pathCStrVec.reserve(path.parts().size() + 1);

    for (const std::string& pathStr : path.parts())
    {
        pathCStrVec.emplace_back(pathStr.c_str());
    }

    pathCStrVec.push_back(nullptr);

    return pathCStrVec;
}

/**
 * @brief Converts a uint128 value to a hexadecimal string representation.
 *
 * This function takes a uint128 value and converts it to a hexadecimal string representation.
 * If MMDB_UINT128_IS_BYTE_ARRAY is defined, it uses the bytesToHexString function to convert
 * the value. Otherwise, it splits the uint128 value into high and low parts and converts them
 * individually to hexadecimal strings.
 *
 * @param uint128 The uint128 value to convert.
 * @return The hexadecimal string representation of the uint128 value.
 */
std::string uint128toHexString(const decltype(MMDB_entry_data_s::uint128)& uint128)
{
    std::string value {"0x"};
#if MMDB_UINT128_IS_BYTE_ARRAY
    value.append(bytesToHexString(uint128, 16));
#else
    uint64_t high = uint128 >> 64;
    uint64_t low = static_cast<uint64_t>(uint128);

    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << high << std::setw(16) << low;
    value.append(ss.str());
#endif
    return value;
}

/**
 * @brief Converts an array of bytes to a hexadecimal string.
 *
 * @param bytes The array of bytes to convert.
 * @param length The length of the array.
 * @return The hexadecimal string representation of the bytes (without spaces or prefixes like "0x").
 */
std::string bytesToHexString(const uint8_t* bytes, size_t length)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i)
    {
        ss << std::setw(2) << static_cast<unsigned>(bytes[i]);
    }
    return ss.str();
}

MMDB_entry_data_list_s*
dumpEntryDataList(MMDB_entry_data_list_s* eDataList, json::Json& jDump, const std::string& path);

/**
 * @brief Dumps the contents of a map entry in the MMDB database to a JSON object.
 *
 * This function takes a linked list of MMDB entry data representing a map entry in the
 * MMDB database, and dumps its contents to a JSON object. The dumped data is stored in
 * the provided JSON object `jDump` under the specified `path`.
 *
 * @param eDataList The linked list of MMDB entry data representing the map entry.
 * @param jDump The JSON object to store the dumped data.
 * @param path The path where the dumped data should be stored in the JSON object.
 * @return The next node in the linked list after the map entry has been dumped.
 * @throws std::runtime_error if the provided entry data list is null or the entry data type is not MMDB_DATA_TYPE_MAP.
 * @throws std::runtime_error if an error occurs while dumping the map entry data.
 */
MMDB_entry_data_list_s* dumpMap(MMDB_entry_data_list_s* eDataList, json::Json& jDump, const std::string& path)
{
    if (eDataList == nullptr || eDataList->entry_data.type != MMDB_DATA_TYPE_MAP)
    {
        throw std::runtime_error {"Error dumping map"};
    }

    jDump.setObject(path);

    uint32_t size = eDataList->entry_data.data_size;
    for (eDataList = eDataList->next; size && eDataList; size--)
    {

        if (MMDB_DATA_TYPE_UTF8_STRING != eDataList->entry_data.type)
        {
            throw std::runtime_error {fmt::format("Error dumping map: {}", MMDB_strerror(MMDB_INVALID_DATA_ERROR))};
        }
        std::string fullKey =
            path + "/" + std::string(eDataList->entry_data.utf8_string, eDataList->entry_data.data_size);

        eDataList = eDataList->next;
        eDataList = dumpEntryDataList(eDataList, jDump, fullKey);
    }

    return eDataList;
}

/**
 * @brief Dumps the array data from MMDB_entry_data_list_s to a JSON object.
 *
 * This function dumps the array data from the given MMDB_entry_data_list_s
 * to a JSON object specified by the provided path. It iterates through the
 * linked list and recursively calls dumpEntryDataList to dump each entry data.
 *
 * @param eDataList The MMDB_entry_data_list_s to dump.
 * @param jDump The JSON object to dump the array data into.
 * @param path The path to the array in the JSON object.
 * @return The next MMDB_entry_data_list_s node after the dumped array.
 * @throws std::runtime_error if the provided eDataList is nullptr or the entry data type is not MMDB_DATA_TYPE_ARRAY.
 */
MMDB_entry_data_list_s* dumpArray(MMDB_entry_data_list_s* eDataList, json::Json& jDump, const std::string& path)
{
    if (eDataList == nullptr || eDataList->entry_data.type != MMDB_DATA_TYPE_ARRAY)
    {
        throw std::runtime_error {fmt::format("Error dumping array: {}", MMDB_strerror(MMDB_INVALID_DATA_ERROR))};
    }

    jDump.setArray(path);

    uint32_t size = eDataList->entry_data.data_size;
    uint32_t index = 0;
    for (eDataList = eDataList->next; size && eDataList; size--)
    {
        std::string fullKey = path + "/" + std::to_string(index++);
        eDataList = dumpEntryDataList(eDataList, jDump, fullKey);
    }

    return eDataList;
}

/**
 * @brief Dumps the MMDB entry data list to a JSON object.
 *
 * This function iterates over the MMDB entry data list and converts each data type
 * to its corresponding JSON representation. The resulting JSON object is stored in
 * the provided `jDump` object at the specified `path`.
 *
 * @param eDataList The MMDB entry data list to be dumped.
 * @param jDump The JSON object to store the dumped data.
 * @param path The path within the JSON object to store the dumped data.
 * @return The updated MMDB entry data list after dumping the current node.
 * @throws std::runtime_error if an error occurs while dumping the entry data list.
 */
MMDB_entry_data_list_s* dumpEntryDataList(MMDB_entry_data_list_s* eDataList, json::Json& jDump, const std::string& path)
{
    switch (eDataList->entry_data.type)
    {
        case MMDB_DATA_TYPE_MAP: eDataList = dumpMap(eDataList, jDump, path); break;
        case MMDB_DATA_TYPE_ARRAY: eDataList = dumpArray(eDataList, jDump, path); break;
        case MMDB_DATA_TYPE_UTF8_STRING:
            jDump.setType(path, std::string(eDataList->entry_data.utf8_string, eDataList->entry_data.data_size));
            eDataList = eDataList->next;
            break;
        case MMDB_DATA_TYPE_BYTES:
            jDump.setType(path, bytesToHexString(eDataList->entry_data.bytes, eDataList->entry_data.data_size));
            eDataList = eDataList->next;
            break;
        case MMDB_DATA_TYPE_DOUBLE:
            jDump.setType(path, eDataList->entry_data.double_value);
            eDataList = eDataList->next;
            break;
        case MMDB_DATA_TYPE_FLOAT:
            jDump.setType(path, eDataList->entry_data.float_value);
            eDataList = eDataList->next;
            break;
        case MMDB_DATA_TYPE_UINT16:
            jDump.setType(path, eDataList->entry_data.uint16);
            eDataList = eDataList->next;
            break;
        case MMDB_DATA_TYPE_UINT32:
            jDump.setType(path, eDataList->entry_data.uint32);
            eDataList = eDataList->next;
            break;
        case MMDB_DATA_TYPE_BOOLEAN:
            jDump.setType(path, eDataList->entry_data.boolean);
            eDataList = eDataList->next;
            break;
        case MMDB_DATA_TYPE_UINT64:
            jDump.setType(path, std::to_string(eDataList->entry_data.uint64));
            eDataList = eDataList->next;
            break;
        case MMDB_DATA_TYPE_UINT128:
            jDump.setType(path, uint128toHexString(eDataList->entry_data.uint128));
            eDataList = eDataList->next;
            break;
        case MMDB_DATA_TYPE_INT32:
            jDump.setType(path, eDataList->entry_data.int32);
            eDataList = eDataList->next;
            break;
        default:
            throw std::runtime_error {
                fmt::format("Error dumping entry data list: {}", MMDB_strerror(MMDB_INVALID_DATA_ERROR))};
    }

    return eDataList;
}

} // namespace

namespace geo
{

base::OptError Locator::lookup(std::string_view ip, const std::shared_ptr<DbEntry>& entry)
{

    if (ip == cachedIp_)
    {
        return base::noError();
    }

    // Lookup the IP address in the db
    int gai_error{0}, mmdb_error{0};
    MMDB_lookup_result_s result =
        MMDB_lookup_string(entry->mmdb.get(), ip.data(), &gai_error, &mmdb_error);
    
    if (0 != gai_error)
    {
        return base::Error{
            fmt::format(
                "{} {}: {}",
                TRANSLATE_ERROR,
                ip,
                gai_strerror(gai_error)

            )
        };
    }

    if (MMDB_SUCCESS != mmdb_error)
    {
        return base::Error{
            fmt::format(
                "{} {}",
                LIBMD_ERROR,
                MMDB_strerror(mmdb_error)
            )
        };
    }

    cachedIp_ = ip;
    cachedResult_ = result;

    return base::noError();
}


base::RespOrError<MMDB_entry_data_s> Locator::getEData(const DotPath& path)
{
    if (!cachedResult_.found_entry)
    {
        return base::Error{"No data found for the IP address"};
    }

    MMDB_entry_data_s eData;
    auto pathCStrVec = getPathCStrVec(path);

    int status = MMDB_aget_value(
        static_cast<MMDB_entry_s* const>(&cachedResult_.entry),
        &eData,
        pathCStrVec.data()
    );

    if (MMDB_SUCCESS != status)
    {   
        return base::Error{
            fmt::format(
                "Error getting value: {}",
                MMDB_strerror(status)
            )
        };
    }

    return eData;
}

base::RespOrError<std::string> Locator::getString(std::string_view ip, const DotPath& path)
{
    auto entry = weakDbEntry_.lock();
    if (nullptr == entry)
    {
        return base::Error{"Database is not available"};
    }

    std::shared_lock lock(entry->rwMutex);

    // Check liveness again; if count == 1 we could be in shutdown or invalid state
    if (1 >= entry.use_count())
    {
        return base::Error{"Database is not available"};
    }

    auto lookError = lookup(ip, entry);
    if (base::isError(lookError))
    {
        return base::getError(lookError);
    }

    auto eDataResp = getEData(path);
    if (base::isError(eDataResp))
    {
        return base::getError(eDataResp);
    }

    auto& eData = base::getResponse(eDataResp);

    if (eData.type != MMDB_DATA_TYPE_UTF8_STRING)
    {
        return base::Error{"Data is not a string"};
    }

    return std::string{eData.utf8_string, eData.data_size};
}

base::RespOrError<std::uint32_t> Locator::getUint32(std::string_view ip, const DotPath& path)
{
    auto entry = weakDbEntry_.lock();
    if (nullptr == entry)
    {
        return base::Error{"Database is not available"};
    }

    std::shared_lock lock(entry->rwMutex);

    // Check liveness again; if count == 1 we could be in shutdown or invalid state
    if (1 >= entry.use_count())
    {
        return base::Error{"Database is not available"};
    }

    auto lookError = lookup(ip, entry);
    if (base::isError(lookError))
    {
        return base::getError(lookError);
    }

    auto eDataResp = getEData(path);
    if (base::isError(eDataResp))
    {
        return base::getError(eDataResp);
    }

    auto& eData = base::getResponse(eDataResp);

    if (eData.type != MMDB_DATA_TYPE_UINT32)
    {
        return base::Error{"Data is not a uint32_t"};
    }

    return eData.uint32;
}

base::RespOrError<double> Locator::getDouble(std::string_view ip, const DotPath& path)
{
    auto entry = weakDbEntry_.lock();
    if (nullptr == entry)
    {
        return base::Error{"Database is not available"};
    }

    std::shared_lock lock(entry->rwMutex);

    // Check liveness again; if count == 1 we could be in shutdown or invalid state
    if (1 >= entry.use_count())
    {
        return base::Error{"Database is not available"};
    }

    auto lookError = lookup(ip, entry);
    if (base::isError(lookError))
    {
        return base::getError(lookError);
    }

    auto eDataResp = getEData(path);
    if (base::isError(eDataResp))
    {
        return base::getError(eDataResp);
    }

    auto& eData = base::getResponse(eDataResp);

    if (eData.type != MMDB_DATA_TYPE_DOUBLE)
    {
        return base::Error{"Data is not a double"};
    }

    return eData.double_value;
}

base::RespOrError<json::Json> Locator::getAsJson(std::string_view ip, const DotPath& path)
{
    auto entry = weakDbEntry_.lock();

    if (nullptr == entry)
    {
        return base::Error{"Database is not available"};
    }

    std::shared_lock lock(entry->rwMutex);

    // Check liveness again; if count == 1 we could be in shutdown or invalid state
    if (1 >= entry.use_count())
    {
        return base::Error{"Database is not available"};
    }

    auto lookError = lookup(ip, entry);
    if (base::isError(lookError))
    {
        return base::getError(lookError);
    }

    auto eDataResp = getEData(path);
    if (base::isError(eDataResp))
    {
        return base::getError(eDataResp);
    }

    auto& eData = base::getResponse(eDataResp);

    json::Json result{};

    switch (eData.type)
    {
        case MMDB_DATA_TYPE_MAP:
        case MMDB_DATA_TYPE_ARRAY:
            return base::Error{"MMDB_DATA_TYPE is not a supported type"};
        case MMDB_DATA_TYPE_UTF8_STRING:
            result.setType("", std::string{eData.utf8_string, eData.data_size});
            break;
        case MMDB_DATA_TYPE_BYTES:
            result.setType("", bytesToHexString(eData.bytes, eData.data_size));
            break;
        case MMDB_DATA_TYPE_DOUBLE:
            result.setType("", eData.double_value);
            break;
        case MMDB_DATA_TYPE_FLOAT:
            result.setType("", eData.float_value);
            break;
        case MMDB_DATA_TYPE_UINT16:
            result.setType("", eData.uint16);
            break;
        case MMDB_DATA_TYPE_UINT32:
            result.setType("", eData.uint32);
            break;
        case MMDB_DATA_TYPE_BOOLEAN:
            result.setType("", eData.boolean);
            break;
        case MMDB_DATA_TYPE_UINT64:
            result.setType("", std::to_string(eData.uint64));
            break;
        case MMDB_DATA_TYPE_UINT128:
            result.setType("", uint128toHexString(eData.uint128));
            break;
        case MMDB_DATA_TYPE_INT32:
            result.setType("", eData.int32);
            break;
        default:
            break;
    }

    return result;
}

} // namespace geo
