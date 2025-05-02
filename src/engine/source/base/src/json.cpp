#include <unordered_set>

#include "base/json.hpp"

#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/schema.h>

namespace json
{

JsonDOM::JsonDOM(rapidjson::Document&& document)
    : document_{std::move(document)}
{

}

JsonDOM::JsonDOM(std::string_view json)
    : document_{}
{
    document_.Parse(json.data());
}

JsonDOM::JsonDOM(const char* json)
    : document_{}
{
    document_.Parse(json);
}


JsonDOM::JsonDOM(const rapidjson::Value & value)
    : document_{}
{
    document_.CopyFrom(value, document_.GetAllocator());
}

JsonDOM::JsonDOM(std::initializer_list<std::pair<std::string_view, JsonValue>> items)
    : document_{}
{
    setTypeMany(items);
}

JsonDOM::JsonDOM(const JsonDOM& other)
    : document_{}
{
    document_.CopyFrom(other.document_, document_.GetAllocator());
}

std::optional<base::Error> JsonDOM::getParseError() const
{
    if (!document_.HasParseError()) { return std::nullopt; }

    const rapidjson::ParseErrorCode error_code = document_.GetParseError();
    const size_t error_offset = document_.GetErrorOffset();
    const char *error_message = rapidjson::GetParseError_En(error_code);

    return base::Error{fmt::format(
        "Parse error at offset {}: {}", error_offset, error_message
    )};
}

bool JsonDOM::exists(std::string_view ptrPath) const
{
    const auto field_ptr = rapidjson::Pointer(ptrPath.data());

    if (!field_ptr.IsValid())
    {
        throw std::runtime_error(fmt::format("..", __func__, ptrPath));
    }

    return field_ptr.Get(document_) != nullptr;

}

bool JsonDOM::equals(std::string_view ptrPath, const JsonDOM& value) const
{
    const auto field_ptr = rapidjson::Pointer(ptrPath.data());

    validatePointer(field_ptr, ptrPath);

    const auto field_value{field_ptr.Get(document_)};
    return (field_value && (*field_value == value.document_));
}

bool JsonDOM::equals(std::string_view basePtrPath, std::string_view referencePtrPath) const
{
    const auto base_ptr = rapidjson::Pointer(basePtrPath.data());
    const auto reference_ptr = rapidjson::Pointer(referencePtrPath.data());

    validatePointer(base_ptr, basePtrPath);
    validatePointer(reference_ptr, referencePtrPath);

    const auto base_value{base_ptr.Get(document_)};
    const auto reference_value{reference_ptr.Get(document_)};

    return (base_value && reference_value && (*base_value == *reference_value));
}

void JsonDOM::set(std::string_view ptrPath, const JsonDOM& value)
{
    const auto field_ptr = rapidjson::Pointer(ptrPath.data());

    validatePointer(field_ptr, ptrPath);

    rapidjson::Value copy;

    copy.CopyFrom(value.document_, document_.GetAllocator());

    field_ptr.Set(document_, copy, document_.GetAllocator());
}

void JsonDOM::set(std::string_view basePtrPath, std::string_view referencePtrPath)
{
    const auto base_ptr      = rapidjson::Pointer(basePtrPath.data());
    const auto reference_ptr = rapidjson::Pointer(referencePtrPath.data());

    validatePointer(base_ptr, basePtrPath);
    validatePointer(reference_ptr, referencePtrPath);

    const auto* reference = reference_ptr.Get(document_);

    if (reference)
    {
        base_ptr.Set(document_, *reference, document_.GetAllocator());
    }
    else
    {
        base_ptr.Set(document_, rapidjson::Value(), document_.GetAllocator());
    }
}

std::optional<std::string> JsonDOM::getString(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    return (value && value->IsString())
        ? std::optional{value->GetString()} : std::nullopt;
}

std::optional<bool> JsonDOM::getBool(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    return (value && value->IsBool())
        ? std::optional{value->GetBool()} : std::nullopt;
}


std::optional<std::int32_t> JsonDOM::getInt32(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    return (value && value->IsInt())
        ? std::optional{value->GetInt()} : std::nullopt;
}

std::optional<std::uint32_t> JsonDOM::getUint32(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    return (value && value->IsUint())
        ? std::optional{value->GetUint()} : std::nullopt;
}

std::optional<std::int64_t> JsonDOM::getInt64(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    return (value && value->IsInt64())
        ? std::optional{value->GetInt64()} : std::nullopt;
}

std::optional<std::uint64_t> JsonDOM::getUint64(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    return (value && value->IsUint64())
        ? std::optional{value->GetUint64()} : std::nullopt;
}

std::optional<std::int64_t> JsonDOM::getIntAsInt64(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    if (value && value->IsInt64())
        return value->GetInt64();
    else if (value && value->IsInt())
        return static_cast<std::int64_t>(value->GetInt());

    return std::nullopt;
}

std::optional<std::uint64_t> JsonDOM::getUintAsUint64(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    if (value && value->IsUint64())
        return value->GetUint64();
    else if (value && value->IsUint())
        return static_cast<std::uint64_t>(value->GetInt());

    return std::nullopt;
}

std::optional<float> JsonDOM::getFloat(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    return (value && value->IsFloat())
        ? std::optional{value->GetFloat()} : std::nullopt;
}

std::optional<double> JsonDOM::getDouble(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    return (value && value->IsDouble())
        ? std::optional{value->GetDouble()} : std::nullopt;
}

std::optional<std::vector<JsonDOM>> JsonDOM::getArray(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    if (!value || !value->IsArray()) { return std::nullopt; }

    const auto& array = value->GetArray();
    std::vector<JsonDOM> result;
    result.reserve(array.Size());

    for (const auto& item : array)
    {
        result.emplace_back( JsonDOM(item) );
    }

    return result;
}

std::optional<std::vector<std::pair<std::string,JsonDOM>>>
    JsonDOM::getObject(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    if (!value || !value->IsObject()) { return std::nullopt; }

    const auto& object = value->GetObject();
    std::vector<std::pair<std::string, JsonDOM>> result;
    result.reserve(object.MemberCount());

    for (const auto& m : object)
    {
        result.emplace_back(
            std::string(m.name.GetString()),
            JsonDOM(m.value)
        );
    }

    return result;
}

static void collectFields(const rapidjson::Value& value,
                    std::string path,
                    std::vector<std::string>& out)
{
    for (const auto& member : value.GetObject())
    {
        std::string new_path
            = path.empty() ? member.name.GetString() : path + "." + member.name.GetString();

        if (member.value.IsObject())
        {
            collectFields(member.value, new_path, out);
        }
        else
        {
            out.push_back(new_path);
        }
    }
}

std::optional<std::vector<std::string>> JsonDOM::getFields() const
{
    if (!document_.IsObject()) { return std::nullopt; }

    std::vector<std::string> result;

    collectFields(document_, "", result);

    return result;
}

std::optional<JsonDOM> JsonDOM::getJsonDOM(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    auto* value = path_ptr.Get(document_);

    if (!value) { return std::nullopt; }

    return std::optional<JsonDOM>{JsonDOM(*value)};
}

std::string JsonDOM::toStrPretty() const
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document_.Accept(writer);
    return buffer.GetString();
}

std::string JsonDOM::toStr() const
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document_.Accept(writer);
    return buffer.GetString();
}

std::optional<std::string> JsonDOM::toStr(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());
    
    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    if (!value) { return std::nullopt; }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value->Accept(writer);

    return buffer.GetString();
}

static bool isTypeHelper(const rapidjson::Value *value, json::Type type)
{
    switch (type)
    {
        case json::Type::Null:      return value->IsNull();
        case json::Type::Object:    return value->IsObject();
        case json::Type::Array:     return value->IsArray();
        case json::Type::String:    return value->IsString();
        case json::Type::Boolean:   return value->IsBool();
        case json::Type::Number:    return value->IsNumber();
        case json::Type::Int:       return value->IsInt();
        case json::Type::Uint:      return value->IsUint();
        case json::Type::Int64:     return value->IsInt64();
        case json::Type::Uint64:    return value->IsUint64();
        case json::Type::Float:     return value->IsFloat();
        case json::Type::Double:    return value->IsDouble();
        case json::Type::Unknown:
        default:
            break;
    }

    throw std::runtime_error(
        fmt::format(
            "Unknown json::Type"
        )
    );
}

bool JsonDOM::isType(std::string_view path, json::Type type) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);
    
    const auto* value = path_ptr.Get(document_);

    if (!value) return false;

    // This function can throw a runtime error
    // If an invalid type is passed
    return isTypeHelper(value,type);
}

bool JsonDOM::isEmpty(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto* value = path_ptr.Get(document_);

    if (!value) { return false; }

    if (isTypeHelper(value, json::Type::Array))
    {
        return value->Empty();
    }
    if (isTypeHelper(value, json::Type::Object))
    {
        return value->ObjectEmpty();
    }
    if (isTypeHelper(value, json::Type::Null))
    {
        return true;
    }
    
    throw std::runtime_error(
        fmt::format(
            "Field '{}' is not an array, object, or null.",
            path
        )
    );
}

size_t JsonDOM::size(std::string_view path) const
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    const auto value = path_ptr.Get(document_);

    if (!value)
    {
        throw std::runtime_error(fmt::format(INVALID_PTR_TYPE_MSG, path));
    }

    if (value->IsArray())
    {
        return value->Size();
    }
    else if (value->IsObject())
    {
        return value->MemberCount();
    }
    else if (value->IsString())
    {
        return value->GetStringLength();
    }
    else
    {
        throw std::runtime_error(fmt::format("Size of field '{}' is not measurable.", path));
    }
}

void JsonDOM::setNull(std::string_view path)
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    path_ptr.Set(document_, rapidjson::Value().SetNull(), document_.GetAllocator());
}

void JsonDOM::setEmpty(std::string_view path)
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    path_ptr.Set(document_, rapidjson::Value(), document_.GetAllocator());
}

void JsonDOM::setObject(std::string_view path)
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    path_ptr.Set(document_, rapidjson::Value().SetObject(), document_.GetAllocator());
}

rapidjson::Value& JsonDOM::setAndGetObject(std::string_view path)
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    auto& allocator = document_.GetAllocator();

    rapidjson::Value obj(rapidjson::kObjectType);

    path_ptr.Set(document_, obj, allocator);

    return *path_ptr.Get(document_);
}

void JsonDOM::setArray(std::string_view path)
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    auto& allocator = document_.GetAllocator();

    path_ptr.Set(document_, rapidjson::Value().SetArray(), allocator);
}

rapidjson::Value& JsonDOM::setAndGetArray(std::string_view path)
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    auto& allocator = document_.GetAllocator();

    rapidjson::Value obj(rapidjson::kArrayType);

    path_ptr.Set(document_, obj, allocator);

    return *path_ptr.Get(document_);
}

void JsonDOM::appendString(std::string_view path, std::string_view value)
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    rapidjson::Value rapidValue{};
    rapidValue.SetString(value.data(), static_cast<rapidjson::SizeType>(value.size()), document_.GetAllocator());

    auto* val = path_ptr.Get(document_);
    if (val)
    {
        if (!val->IsArray())
        {
            val->SetArray();
        }

        val->PushBack(rapidValue, document_.GetAllocator());
    }
    else
    {
        rapidjson::Value vArray;
        vArray.SetArray();
        vArray.PushBack(rapidValue, document_.GetAllocator());
        path_ptr.Set(document_, vArray, document_.GetAllocator());
    }
}

void JsonDOM::appendJson(std::string_view path, const JsonDOM& value)
{
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    rapidjson::Value rapidValue;
    rapidValue.CopyFrom(value.document_, document_.GetAllocator());

    auto* val = path_ptr.Get(document_);
    if (val)
    {
        if (!val->IsArray())
        {
            val->SetArray();
        }

        val->PushBack(rapidValue, document_.GetAllocator());
    }
    else
    {
        rapidjson::Value vArray;
        vArray.SetArray();
        vArray.PushBack(rapidValue, document_.GetAllocator());
        path_ptr.Set(document_, vArray, document_.GetAllocator());
    }
}

bool JsonDOM::erase(std::string_view path)
{
    if (path.empty())
    {
        document_.SetNull();
        return true;
    }
    
    const auto path_ptr = rapidjson::Pointer(path.data());

    validatePointer(path_ptr, path);

    return path_ptr.Erase(document_);
}

std::optional<base::Error> JsonDOM::validate(const JsonDOM& schema) const
{
    rapidjson::SchemaDocument schema_doc{schema.document_};
    rapidjson::SchemaValidator validator{schema_doc};

    if (!document_.Accept(validator))
    {
        rapidjson::StringBuffer sb;

        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        std::string schema_path = sb.GetString();
        sb.Clear();

        std::string keyword = validator.GetInvalidSchemaKeyword();
        sb.Clear();

        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        std::string document_path = sb.GetString();

        return base::Error {
            fmt::format(
                "Schema validation failed: "
                "Invalid schema Keyword: '{}'. "
                "Schema path: '{}', "
                "Document path: '{}'\n",
                keyword,
                schema_path,
                document_path
            )
        };
    }

    return std::nullopt;
}

std::optional<base::Error> Json::checkDuplicateKeys() const {
    struct Checker {
        std::optional<base::Error> error;
        void visit(const rapidjson::Value& v) {
            if (!v.IsObject() || error) return;
            std::unordered_set<std::string_view> seen;
            for (auto it = v.MemberBegin(); it != v.MemberEnd(); ++it) {
                auto key = std::string_view(it->name.GetString());
                if (!seen.insert(key).second) {
                    error = base::Error{
                      fmt::format("Duplicate JSON key '{}'", key)
                    };
                    return;
                }
            }
            for (auto it = v.MemberBegin(); it != v.MemberEnd(); ++it)
                visit(it->value);
        }
    } checker;
    checker.visit(document_);
    return checker.error;
}

std::string JsonDOM::formatJsonPath(std::string_view dotPath, bool skipDot)
{
    std::string ptrPath {dotPath};

    if ("." == ptrPath)
    {
        ptrPath = "";
    }
    else
    {
        // Replace ~ with ~0
        for (auto pos = ptrPath.find('~'); 
                pos != std::string::npos;
                pos = ptrPath.find('~', pos + 2))
        {
            ptrPath.replace(pos, 1, "~0");
        }

        // Replace / with ~1
        for (auto pos = ptrPath.find('/');
                pos != std::string::npos;
                pos = ptrPath.find('/', pos + 2))
        {
            ptrPath.replace(pos, 1, "~1");
        }

        // Replace . with /
        if (!skipDot)
        {
            std::string result;
            result.reserve(ptrPath.size());
            bool prevCharWasSlash = false;

            for (char c : ptrPath)
            {
                if (c == '.' && !prevCharWasSlash)
                {
                    result += '/';
                }
                else if (c != '\\' || ((c == '.' || c == '\\') && prevCharWasSlash))
                {
                    result += c;
                }
                prevCharWasSlash = (c == '\\');
            }
            ptrPath = std::move(result);
        }

        // Add / at the beginning
        if (ptrPath.front() != '/')
        {
            ptrPath.insert(0, "/");
        }
    }

    return ptrPath;
}

} // namespace json
