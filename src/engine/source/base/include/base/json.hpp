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

//const char* GetPointerParseError_En(rapidjson::PointerParseErrorCode code)
//{
//    switch (code) {
//        case rapidjson::kPointerParseErrorNone:
//            return "No error.";
//        case rapidjson::kPointerParseErrorTokenMustBeginWithSolidus:
//            return "A token must begin with a '/'.";
//        case rapidjson::kPointerParseErrorInvalidEscape:
//            return "Invalid escape sequence.";
//        case rapidjson::kPointerParseErrorInvalidPercentEncoding:
//            return "Invalid percent encoding in URI fragment.";
//        case rapidjson::kPointerParseErrorCharacterMustPercentEncode:
//            return "Character must be percent-encoded in URI fragment.";
//        default:
//            return "Unknown pointer parse error.";
//    }
//}

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

} // namespace

enum Type {
    Null = 0,
    Object,
    Array,
    String,
    Number,
    Int,
    Int64,
    Double,
    Float,
    Boolean,
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
    
    std::optional<base::Error> getError() const;

    bool exists(std::string_view ptrPath) const;

    bool equals(std::string_view ptrPath, const JsonDOM& value) const;

    bool equals(std::string_view basePtrPath, std::string_view referencePtrPath) const;

    void set(std::string_view ptrPath, const JsonDOM& value);

    void set(std::string_view basePtrPath, std::string_view referencePtrPath);

    std::optional<std::string> getString(std::string_view path) const;

    std::optional<std::int32_t> getInt32(std::string_view path) const;

    std::optional<std::int64_t> getInt64(std::string_view path) const;

    std::optional<std::int64_t> getIntAsInt64(std::string_view path) const;

    std::optional<float> getFloat(std::string_view path) const;

    std::optional<double> getDouble(std::string_view path) const;

    std::optional<double> getNumberAsDouble(std::string_view path) const;

    std::optional<bool> getBool(std::string_view path) const;

    std::optional<std::vector<JsonDOM>> getArray(std::string_view path) const;

    std::optional<std::vector<std::pair<std::string,JsonDOM>>>
        getObject(std::string_view) const;

    std::optional<std::vector<std::string>> getFields() const;

    std::optional<JsonDOM> getJsonDOM(std::string_view path) const;

    std::string toStrPretty() const;

    std::string toStr() const;

    std::optional<std::string> toStr(std::string_view path) const;

    bool isType(std::string_view path, json::Type type) const;

    bool isEmpty(std::string_view path, json::Type type) const;

    size_t size(std::string_view path) const;

    template <typename T>
    void setType(T&& value, json::Type type, std::string_view path)
    {
        const auto path_ptr = rapidjson::Pointer(path.data());

        validatePointer(path_ptr, path);

        switch (type)
        {
            case json::Type::Null:
                path_ptr.Set(document_, rapidjson::Value().SetNull());
                break;
            case json::Type::Object: 
                path_ptr.Set(document_, rapidjson::Value().SetObject());
                break;
            case json::Type::Array:
                path_ptr.Set(document_, rapidjson::Value().SetArray());
                break;
            case json::Type::String:
            case json::Type::Number:
            case json::Type::Int:
            case json::Type::Int64:
            case json::Type::Double:
            case json::Type::Float:
            case json::Type::Boolean:
                path_ptr.Set(document_, std::forward<T>(value));
                break;
            case json::Type::Unknown:
                break;
        }
    }

    bool erase(std::string_view path);

    std::optional<base::Error> validate(const JsonDOM& schema) const;


    static std::string formatJsonPath(std::string_view dotPath, bool skipDot);

private:

    rapidjson::Document document_;
};


using Json = JsonDOM;

} // namespace json

#endif // _JSON_HPP
