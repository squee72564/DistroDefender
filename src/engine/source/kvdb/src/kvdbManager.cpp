#include <filesystem>
#include <fstream>
#include <optional>

#include <fmt/format.h>

#include <rocksdb/db.h>
#include <rocksdb/options.h>

#include <base/logger.hpp>
#include <kvdb/kvdbManager.hpp>

namespace kvdbManager
{

KVDBManager::KVDBManager(const KVDBManagerOptions& options)
    : kvdbHandlerCollection_{std::make_shared<KVDBHandlerCollection>()}
    , managerOptions_{options}
{}

void KVDBManager::initialize()
{
    if (!isInitialized_)
    {
        initializeOptions();
        initializeMainDB();
        isInitialized_ = true;
    }
}

void KVDBManager::finalize()
{
    if (isInitialized_)
    {
        finalizeMainDB();
        isInitialized_ = false;
    }
}

std::map<std::string, RefInfo> KVDBManager::getKVDBScopesInfo()
{
    // List reverse lookup of getKVDBHandlersInfo. List of scopes and DBs that are using them.
    std::map<std::string, kvdbManager::RefInfo> retValue;

    // Retrieve the list of DBs and scopes that are using them.
    std::map<std::string, kvdbManager::RefInfo> handlersInfo = getKVDBHandlersInfo();

    // Create a temporal map with the reverse lookup indexed by Scope instead of Database.
    std::map<std::string, kvdbManager::RefCounter> refCounterMap;

    // Iterate over the map of DBs and scopes that are using them.
    for (const auto& [dbName, scopesUsingDB] : handlersInfo)
    {
        // Iterate over the scopes that are using thisDB.
        for (const auto& [scopeName, countDBsUsingScope] : scopesUsingDB)
        {
            // Get the current refCounter for this scope.
            auto& counterMap = refCounterMap[scopeName];

            // Insert number of used DBs in current scope.
            counterMap.addRef(dbName, countDBsUsingScope);

            // Update the refCounter for this scope.
            refCounterMap[scopeName] = counterMap;
        }
    }

    for (auto& entry : refCounterMap)
    {
        const auto& scopeName = entry.first;
        const auto& refCounter = entry.second;
        const auto& refInfo = refCounter.getRefMap();
        retValue.emplace(scopeName, refInfo);
    }

    return retValue;
}

std::map<std::string, RefInfo> KVDBManager::getKVDBHandlersInfo() const
{
    std::map<std::string, kvdbManager::RefInfo> retValue;
    auto dbNames = kvdbHandlerCollection_->getDBNames();

    for (const auto& dbName : dbNames)
    {
        auto refInfo = kvdbHandlerCollection_->getRefMap(dbName);
        retValue.insert({dbName, refInfo});
    }

    return retValue;
}

uint32_t KVDBManager::getKVDBHandlersCount(const std::string& dbName) const
{
    const auto handlersInfo = getKVDBHandlersInfo();

    uint32_t retValue{0};

    if (handlersInfo.count(dbName))
    {
        auto scopes = handlersInfo.at(dbName);
        for (const auto& [key, val] : scopes)
        {
            retValue += val;
        }
    }

    return retValue;
}

base::RespOrError<std::shared_ptr<IKVDBHandler>>
KVDBManager::getKVDBHandler(const std::string& dbName, const std::string& scopeName)
{
    std::shared_ptr<rocksdb::ColumnFamilyHandle> cfHandle;

    if (mapCFHandles_.count(dbName))
    {
        cfHandle = mapCFHandles_[dbName];
    }
    else
    {
        return base::Error{
            fmt::format(
                "The DB '{}' does not exist.",
                dbName
            )
        };
    }

    kvdbHandlerCollection_->addKVDBHandler(dbName, scopeName);

    auto kvdbHandler = std::make_shared<KVDBHandler>(pRocksDB_, cfHandle, kvdbHandlerCollection_, dbName, scopeName);

    return kvdbHandler;
}

std::vector<std::string> KVDBManager::listDBs(const bool loaded)
{
    std::vector<std::string> spaces;

    spaces.reserve(mapCFHandles_.size());

    for (const auto& cf : mapCFHandles_)
    {
        spaces.push_back(cf.first);
    }

    return spaces;
}

base::OptError KVDBManager::deleteDB(const std::string& name)
{
    const auto refCount = getKVDBHandlersCount(name);

    if (refCount)
    {
        return base::Error{
            fmt::format(
                "Could not remove the DB '{}'. Usage reference count: {}",
                name,
                refCount
            )
        };
    }

    auto it = mapCFHandles_.find(name);

    if (it == mapCFHandles_.end())
    {
        return base::Error{
            fmt::format(
                "The DB '{}' does not exist.",
                name
            )
        };
    }

    auto cfHandle = it->second;

    try
    {
        const auto opStatus = pRocksDB_->DropColumnFamily(cfHandle.get());

        if (!opStatus.ok())
        {
            return base::Error{
                fmt::format(
                    "Database '{}' could not be removed: {}.",
                    name, opStatus.ToString()
                )
            };
        }

        mapCFHandles_.erase(it);
    }
    catch (const std::runtime_error& e)
    {
        return base::Error{
            fmt::format(
                "Database '{}' could not be removed: {}.",
                name, 
                e.what()
            )
        };
    }

    return std::nullopt;
}

base::OptError KVDBManager::createDB(const std::string& name)
{
    if (existsDB(name))
    {
        return std::nullopt;
    }

    return createColumnFamily(name);
}

base::OptError KVDBManager::createDB(const std::string& name, const std::string& path)
{
    auto result = getContentFromJsonFile(path);

    if (std::holds_alternative<base::Error>(result))
    {
        return std::get<base::Error>(result);
    }

    auto content{ std::move(std::get<json::Json>(result)) };

    auto errorCreate = createDB(name);

    if (errorCreate)
    {
        return errorCreate;
    }

    auto errorLoad = loadDBFromJson(name, content);

    if (errorLoad)
    {
        auto errorDelete = deleteDB(name);

        if (errorDelete)
        {
            return errorDelete;
        }

        return errorLoad;
    }

    return std::nullopt;
}

base::OptError KVDBManager::loadDBFromJson(const std::string& name, const json::Json& content)
{
    std::vector<std::pair<std::string, json::Json>> entries{};
    std::shared_ptr<rocksdb::ColumnFamilyHandle> cfHandle{};

    if (mapCFHandles_.count(name))
    {
        cfHandle = mapCFHandles_[name];
    }

    if (!cfHandle)
    {
        return base::Error{
            fmt::format(
                "The DB '{}' does not exist.",
                name
            )
        };
    }

    entries = content.getObject().value();

    for (const auto& [key, val] : entries)
    {
        const auto status = pRocksDB_->Put(rocksdb::WriteOptions(), cfHandle.get(), key, val.toStr());
        
        if (!status.ok())
        {
            return base::Error{
                fmt::format(
                    "An error occurred while inserting data key{}, value {}: {}",
                    key,
                    val.toStr(),
                    status.ToString()
                )
            };
        }
    }

    return std::nullopt;
}

bool KVDBManager::existsDB(const std::string& name)
{
    return mapCFHandles_.count(name) > 0;
}

void KVDBManager::initializeOptions()
{
    rocksDBOptions_ = rocksdb::Options();
    rocksDBOptions_.IncreaseParallelism();
    rocksDBOptions_.OptimizeLevelStyleCompaction();
    rocksDBOptions_.create_if_missing = true;
}

void KVDBManager::initializeMainDB()
{
    const auto dbStoragePath = managerOptions_.dbStoragePath.string();

    std::filesystem::create_directories(dbStoragePath);

    const std::string dbNameFullPath{
        fmt::format(
            "{}{}",
            dbStoragePath,
            managerOptions_.dbName
        )
    };

    std::vector<std::string> columnNames;

    std::vector<rocksdb::ColumnFamilyDescriptor> cfDescriptors;
    std::vector<rocksdb::ColumnFamilyHandle*> cfHandles;

    bool hasDefaultCF{false};

    const auto listStatus = rocksdb::DB::ListColumnFamilies(rocksdb::DBOptions(), dbNameFullPath, &columnNames);

    if (listStatus.ok())
    {
        for (const auto& cfName : columnNames)
        {
            if (rocksdb::kDefaultColumnFamilyName == cfName)
            {
                hasDefaultCF = true;
            }

            auto newDescriptor = rocksdb::ColumnFamilyDescriptor(cfName, rocksdb::ColumnFamilyOptions());
            cfDescriptors.push_back(newDescriptor);
        }
    }

    if (!hasDefaultCF)
    {
        auto newDescriptor =
            rocksdb::ColumnFamilyDescriptor(rocksdb::kDefaultColumnFamilyName, rocksdb::ColumnFamilyOptions());

        cfDescriptors.push_back(newDescriptor);
    }

    rocksdb::DB* rawRocksDBPtr{nullptr};

    auto statusOpen = rocksdb::DB::Open(rocksDBOptions_, dbNameFullPath, cfDescriptors, &cfHandles, &rawRocksDBPtr);

    if (statusOpen.ok())
    {
        pRocksDB_ = std::shared_ptr<rocksdb::DB>(rawRocksDBPtr);

        for (std::size_t cfDescriptorIndex = 0; cfDescriptorIndex < cfDescriptors.size(); cfDescriptorIndex++)
        {
            const auto& dbName = cfDescriptors[cfDescriptorIndex].name;

            if (rocksdb::kDefaultColumnFamilyName != dbName)
            {
                mapCFHandles_.emplace(dbName, createSharedCFHandle(cfHandles[cfDescriptorIndex]));
            }
            else
            {
                pDefaultCFHandle_ = createSharedCFHandle(cfHandles[cfDescriptorIndex]);
            }
        }
    }
    else
    {
        throw std::runtime_error(
            fmt::format(
                "An error occurred while trying to open the database: {}",
                statusOpen.ToString()
            )
        );
    }

}

void KVDBManager::finalizeMainDB()
{
    mapCFHandles_.clear();
    pDefaultCFHandle_.reset();
    pRocksDB_.reset();
}   

base::RespOrError<json::Json> KVDBManager::getContentFromJsonFile(const std::string& path)
{
    std::vector<std::tuple<std::string, json::Json>> entries {};

    // TODO: to improve
    if (path.empty())
    {
        return base::Error {"The path is empty."};
    }

    // Open file and read content
    std::string contents;
    // TODO: No check the size, the location, the type of file, the permissions it's a
    // security issue. The API should be changed to receive a stream instead of a path
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (in)
    {
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], static_cast<std::streamsize>(contents.size()));
        in.close();
    }
    else
    {
        return base::Error {fmt::format("An error occurred while opening the file '{}'", path.c_str())};
    }

    json::Json fileContentsJson;

    try
    {
        fileContentsJson = json::Json {contents.c_str()};
    }
    catch (const std::exception& e)
    {
        return base::Error {fmt::format("An error occurred while parsing the JSON file '{}'", path.c_str())};
    }

    if (!fileContentsJson.isType("", json::Type::Object))
    {
        return base::Error {
            fmt::format("An error occurred while parsing the JSON file '{}': JSON is not an object", path.c_str())};
    }

    return fileContentsJson;
}

std::shared_ptr<rocksdb::ColumnFamilyHandle>
KVDBManager::createSharedCFHandle(rocksdb::ColumnFamilyHandle* cfRawPtr)
{
    return std::shared_ptr<rocksdb::ColumnFamilyHandle>(
        cfRawPtr,
        [pRocksDB = pRocksDB_](rocksdb::ColumnFamilyHandle* ptr)
        {
            const auto opStatus = pRocksDB->DestroyColumnFamilyHandle(ptr);
            if (!opStatus.ok())
            {
                throw std::runtime_error(
                    fmt::format(
                        "An error occurred while trying to destroy CF: {}",
                        opStatus.ToString()
                    )
                );
            }
        }
    );
}

base::OptError KVDBManager::createColumnFamily(const std::string& name)
{
    rocksdb::ColumnFamilyHandle* cfHandle{nullptr};
    rocksdb::Status status{pRocksDB_->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), name, &cfHandle)};

    if (!status.ok())
    {
        return base::Error{
            fmt::format(
                "Could not create DB '{}', RocksDB status: {}.",
                name, status.ToString()
            )
        };
    }

    mapCFHandles_.emplace(name, createSharedCFHandle(cfHandle));

    return std::nullopt;
}

} // namespace kvdbManager

