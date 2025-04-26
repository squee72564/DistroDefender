#ifndef _JSON_HPP
#define _JSON_HPP

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include <rapidjson/document.h>
#include <rapidjson/Pointer.h>
#include <rapidjson/error/en.h>

#include <fmt/format.h>

#include <base/error.hpp>


namespace json
{

namespace
{
constexpr auto INVALID_PTR_TYPE_MSG = "INVALID pointer path '{}'";

constexpr auto INVALID_PTR_PARSE_MSG = "Could not parse '{}' at offset {}: {}";

static inline void validatePointer(const rapidjson::Pointer& ptr, std::string_view ptr_path)
{
    if (ptr.IsValid())
    {
        return;
    }

    const size_t error_offset{ptr.GetParseErrorOffset()};
    rapidjson::PointerParseErrorCode error_code = ptr.GetParseErrorCode();

    throw std::runtime_error(
        fmt::format(
            INVALID_PTR_PARSE_MSG,
            ptr_path,
            error_offset,
            GetPointerParseError_En(error_code)
        )
    );
}

template <typename T>
struct is_rapidjson_trivially_settable : std::disjunction<
    std::is_same<std::decay_t<T>, int>,
    std::is_same<std::decay_t<T>, unsigned>,
    std::is_same<std::decay_t<T>, int64_t>,
    std::is_same<std::decay_t<T>, uint64_t>,
    std::is_same<std::decay_t<T>, float>,
    std::is_same<std::decay_t<T>, double>,
    std::is_same<std::decay_t<T>, bool>,
    std::is_same<std::decay_t<T>, const char*>,
    std::is_same<std::decay_t<T>, std::string_view>,
    std::is_same<std::decay_t<T>, std::string>
> {};

template <typename>
struct dependent_false : std::false_type {};

} // namespace

enum Type {
    Null = 0,
    Object,
    Array,
    String,
    Boolean,
    Number,
    Int,
    Uint,
    Int64,
    Uint64,
    Float,
    Double,
    Unknown
};
class JsonDOM
{
public:
    JsonDOM() = default;

    explicit JsonDOM(rapidjson::Document&& document);

    explicit JsonDOM(const char* json);

    explicit JsonDOM(const rapidjson::Value & value);

    JsonDOM(const JsonDOM& other) = delete;

    JsonDOM& operator=(const JsonDOM& other) = delete;

    JsonDOM(JsonDOM&& other) noexcept = default;

    JsonDOM& operator=(JsonDOM&& other) noexcept = default;

    friend bool operator==(const JsonDOM &lhs, const JsonDOM &rhs)
        { return lhs.document_ == rhs.document_; }

    friend bool operator!=(const JsonDOM &lhs, const JsonDOM &rhs)
        { return lhs.document_ != rhs.document_; }
    
    std::optional<base::Error> getParseError() const;

    bool exists(std::string_view ptrPath) const;

    bool equals(std::string_view ptrPath, const JsonDOM& value) const;

    bool equals(std::string_view basePtrPath, std::string_view referencePtrPath) const;

    void set(std::string_view ptrPath, const JsonDOM& value);

    void set(std::string_view basePtrPath, std::string_view referencePtrPath);

    std::optional<std::string> getString(std::string_view path) const;

    std::optional<bool> getBool(std::string_view path) const;

    std::optional<std::int32_t> getInt32(std::string_view path) const;

    std::optional<std::uint32_t> getUint32(std::string_view path) const;

    std::optional<std::int64_t> getInt64(std::string_view path) const;

    std::optional<std::uint64_t> getUint64(std::string_view path) const;

    std::optional<std::int64_t> getIntAsInt64(std::string_view path) const;

    std::optional<std::uint64_t> getUintAsUint64(std::string_view path) const;

    std::optional<float> getFloat(std::string_view path) const;

    std::optional<double> getDouble(std::string_view path) const;

    std::optional<std::vector<JsonDOM>> getArray(std::string_view path) const;

    std::optional<std::vector<std::pair<std::string,JsonDOM>>>
        getObject(std::string_view = "") const;

    std::optional<std::vector<std::string>> getFields() const;

    std::optional<JsonDOM> getJsonDOM(std::string_view path = "") const;

    std::string toStrPretty() const;

    std::string toStr() const;

    std::optional<std::string> toStr(std::string_view path) const;

    bool isType(std::string_view path, json::Type type) const;

    bool isEmpty(std::string_view path) const;

    size_t size(std::string_view path) const;

    void setNull(std::string_view path);

    void setEmpty(std::string_view path);

    void setObject(std::string_view path);

    rapidjson::Value& setAndGetObject(std::string_view path);

    void setArray(std::string_view path);

    rapidjson::Value& setAndGetArray(std::string_view path);

    template <typename T>
    void setType(std::string_view path, T&& value)
    {
        static_assert(
            is_rapidjson_trivially_settable<T>::value,
            "type T must be trivially settable into a rapidjson::Value object"
        );

        const auto path_ptr = rapidjson::Pointer(path.data());

        validatePointer(path_ptr, path);

        rapidjson::Value v;

        if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
            v.SetBool(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, int>) {
            v.SetInt(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, int64_t>) {
            v.SetInt64(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, unsigned>) {
            v.SetUint(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, uint64_t>) {
            v.SetUint64(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, float>) {
            v.SetFloat(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, double>) {
            v.SetDouble(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, const char*>) {
            v.SetString(
                value,
                static_cast<rapidjson::SizeType>(std::strlen(value)),
                document_.GetAllocator()
            );
        } else if constexpr (std::is_same_v<std::decay_t<T>, std::string_view>) {
            v.SetString(
                value.data(),
                static_cast<rapidjson::SizeType>(value.size()),
                document_.GetAllocator()
            );
        } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            v.SetString(
                value.c_str(),
                static_cast<rapidjson::SizeType>(value.size()),
                document_.GetAllocator()
            );
        } else {
            static_assert(dependent_false<T>::value, "Unhandled type in setType");
        }

        path_ptr.Set(document_, v);
    }

    template <typename T>
    rapidjson::Value& setAndGetType(std::string_view path, T&& value)
    {
        static_assert(
            is_rapidjson_trivially_settable<T>::value,
            "type T must be trivially settable into a rapidjson::Value object"
        );

        const auto path_ptr = rapidjson::Pointer(path.data());

        validatePointer(path_ptr, path);

        rapidjson::Value v;

        if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
            v.SetBool(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, int>) {
            v.SetInt(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, int64_t>) {
            v.SetInt64(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, unsigned>) {
            v.SetUint(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, uint64_t>) {
            v.SetUint64(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, float>) {
            v.SetFloat(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, double>) {
            v.SetDouble(value);
        } else if constexpr (std::is_same_v<std::decay_t<T>, const char*>) {
            v.SetString(
                value,
                static_cast<rapidjson::SizeType>(std::strlen(value)),
                document_.GetAllocator()
            );
        } else if constexpr (std::is_same_v<std::decay_t<T>, std::string_view>) {
            v.SetString(
                value.data(),
                static_cast<rapidjson::SizeType>(value.size()),
                document_.GetAllocator()
            );
        } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            v.SetString(
                value.c_str(),
                static_cast<rapidjson::SizeType>(value.size()),
                document_.GetAllocator()
            );
        } else {
            static_assert(dependent_false<T>::value, "Unhandled type in setType");
        }

        path_ptr.Set(document_, v);

        return *path_ptr.Get(document_);
    }

    bool erase(std::string_view path);

    std::optional<base::Error> validate(const JsonDOM& schema) const;

    std::optional<base::Error> checkDuplicateKeys() const;

    static std::string formatJsonPath(std::string_view dotPath, bool skipDot);

private:

    rapidjson::Document document_;
};


using Json = JsonDOM;

} // namespace json

#endif // _JSON_HPP
