#include <type_traits>
#include <iostream>

#include <base/error.hpp>
#include <base/json.hpp>

#include <gtest/gtest.h>

#include <rapidjson/document.h>


namespace {
const char* jsonStr = R"({
    "hello": "world",
    "t": true,
    "f": false,
    "n": null,
    "i": 123,
    "pi": 3.1416,
    "a": [1, 2, 3, 4],
    "t2": true
})";

const char* jsonNestedObj = R"({
    "o": {
        "i": 42
    },
    "o2": {
        "a": 1,
        "b": 2,
        "c": false,
        "d": null
    },
    "test": "testing"
})";

const char* errStr = R"({
    "hello" "world",
    "t": true,
    "f": false,
    "n": null,
    "i": 123,
    "pi": 3.1416,
    "a": [1, 2, 3, 4]
})";
}

TEST(JsonTest, CopyDeleted)
{
    ASSERT_FALSE(std::is_copy_assignable<json::Json>::value);
}

TEST(JsonTest, MoveEnabled)
{
    ASSERT_TRUE(std::is_move_constructible<json::Json>::value);
    ASSERT_TRUE(std::is_move_assignable<json::Json>::value);
}

TEST(JsonTest, DefaultConstructor)
{
    json::Json json{};
    std::optional<base::Error> err = json.getParseError();
    ASSERT_FALSE(base::isError(err));
    ASSERT_TRUE(json.isEmpty(""));
}

TEST(JsonTest, DocumentConstructor)
{
    json::Json json{rapidjson::Document()};
    std::optional<base::Error> err = json.getParseError();
    ASSERT_FALSE(base::isError(err));
    ASSERT_TRUE(json.isEmpty(""));
}

TEST(JsonTest, ValueConstructor)
{
    rapidjson::Value value;
    json::Json json{value};
    std::optional<base::Error> err = json.getParseError();
    ASSERT_FALSE(base::isError(err));
    ASSERT_TRUE(json.isEmpty(""));
}

TEST(JsonTest, CStrConstructor)
{
    json::Json json{jsonStr};
    std::optional<base::Error> err = json.getParseError();
    ASSERT_FALSE(base::isError(err));
    ASSERT_FALSE(json.isEmpty(""));
}

TEST(JsonTest, GetParseError)
{
    json::Json json{errStr};
    std::optional<base::Error> err = json.getParseError();
    ASSERT_TRUE(base::isError(err));
}

TEST(JsonTest, TestExists)
{
    json::Json json{jsonStr};
    std::optional<base::Error> err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.exists("/hello"));
    ASSERT_FALSE(json.exists("/non_existent"));
}

TEST(JsonTest, equalsJsonDOM)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json::Json json2{jsonStr};
    err = json2.getParseError();
    ASSERT_FALSE(base::isError(err));

    json::Json json3{};
    err = json3.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.equals("", json2));
    ASSERT_TRUE(json2.equals("", json));

    ASSERT_FALSE(json.equals("", json3));
}

TEST(JsonTest, equalsPtrPath)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.equals("/t", "/t2"));
    ASSERT_FALSE(json.equals("/t", "/hello"));
}

TEST(JsonTest, setJsonDOM)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json::Json json2{};
    json2.set("", json);

    ASSERT_TRUE(json2.equals("", json));
}

TEST(JsonTest, setPtrPath)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json.set("/t3", "/t");

    ASSERT_TRUE(json.equals("/t", "/t3"));
}

TEST(JsonTest, getString)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.isType("/hello", json::Type::String));
    
    std::optional<std::string> s = json.getString("/hello");

    ASSERT_TRUE(s.has_value());
    ASSERT_TRUE(s.value() == "world");
}

TEST(JsonTest, getBool)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.isType("/f", json::Type::Boolean));
    
    std::optional<bool> val = json.getBool("/f");

    ASSERT_TRUE(val.has_value());
    ASSERT_TRUE(val == false);

    ASSERT_FALSE(json.isType("/hello", json::Type::Boolean));

    val = json.getBool("/hello");
    ASSERT_FALSE(val.has_value());
}

TEST(JsonTest, getInt32)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.isType("/i", json::Type::Int));
    
    std::optional<std::int32_t> val = json.getInt32("/i");

    ASSERT_TRUE(val.has_value());
    ASSERT_TRUE(val.value() == 123);
}

TEST(JsonTest, getUint32)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json.setType("/ui32", static_cast<std::uint32_t>(123));

    ASSERT_TRUE(json.isType("/ui32", json::Type::Uint));
    
    std::optional<std::uint32_t> val = json.getInt32("/ui32");

    ASSERT_TRUE(val.has_value());
    ASSERT_TRUE(val.value() == 123);
}

TEST(JsonTest, getInt64)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.isType("/i", json::Type::Int64));
    
    std::optional<std::int64_t> val = json.getInt64("/i");

    ASSERT_TRUE(val.has_value());
    ASSERT_TRUE(val.value() == 123);
}

TEST(JsonTest, getUint64)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json.setType("/ui64", static_cast<std::uint64_t>(123));

    ASSERT_TRUE(json.isType("/ui64", json::Type::Uint64));
    
    std::optional<std::uint64_t> val = json.getInt32("/ui64");

    ASSERT_TRUE(val.has_value());
    ASSERT_TRUE(val.value() == 123);
}

TEST(JsonTest, getFloat)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.isType("/pi", json::Type::Float));
    
    std::optional<float> val = json.getFloat("/pi");

    ASSERT_TRUE(val.has_value());
    ASSERT_TRUE(val == 3.1416f);
}

TEST(JsonTest, getDouble)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.isType("/pi", json::Type::Double));
    
    std::optional<double> val = json.getDouble("/pi");

    ASSERT_TRUE(val.has_value());
    ASSERT_TRUE(val == 3.1416);
}

TEST(JsonText, getArray)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.isType("/a", json::Type::Array));
    
    std::optional<std::vector<json::Json>> val = json.getArray("/a");

    ASSERT_TRUE(val.has_value());

    std::vector<json::Json> v = std::move(val.value());

    std::int32_t i = 1;
    for (const auto& obj : v) {
        ASSERT_TRUE(i++ == obj.getInt32("").value());
    }

    ASSERT_FALSE(json.isType("/hello", json::Type::Array));

    val = json.getArray("/hello");
    ASSERT_FALSE(val.has_value());
}

TEST(JsonTest, getObject)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonNestedObj};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(json.isType("/o", json::Type::Object));
    
    std::optional<
        std::vector<std::pair<std::string,json::Json>>
    > val = json.getObject("/o");

    ASSERT_TRUE(val.has_value());

    std::vector<std::pair<std::string,json::Json>> o
        = std::move(val.value());
    
    const auto& [key, child] = o[0];

    ASSERT_TRUE(child.isType("", json::Type::Int));
    std::optional<std::int32_t> child_val = child.getInt32("");

    ASSERT_TRUE(child_val.has_value());
    ASSERT_TRUE(42 == child_val.value());

    ASSERT_FALSE(json.isType("/hello", json::Type::Object));

    val = json.getObject("/hello");
    ASSERT_FALSE(val.has_value());
}

TEST(JsonTest, collectFields)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonNestedObj};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    std::optional<std::vector<std::string>> val = json.getFields();

    ASSERT_TRUE(val.has_value());

    std::vector<std::string> fields = val.value();
    std::vector<std::string> expected{
        "o.i",
        "o2.a",
        "o2.b",
        "o2.c",
        "o2.d",
        "test"
    };

    ASSERT_TRUE(fields.size() == expected.size());

    for (int i = 0; i < fields.size(); ++i) {
        ASSERT_TRUE(fields[i] == expected[i]);
    }
}

TEST(JsonTest, getJsonDOM)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonNestedObj};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    std::optional<json::Json> val = json.getJsonDOM("");

    ASSERT_TRUE(val.has_value());

    json::Json newJson = std::move(val.value());

    ASSERT_TRUE(newJson.equals("", json));
}

TEST(JsonTest, size)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));
    
    ASSERT_TRUE(4 == json.size("/a"));
    ASSERT_TRUE(5 == json.size("/hello"));

    json::Json jsonObj{jsonNestedObj};
    err = jsonObj.getParseError();
    ASSERT_FALSE(base::isError(err));

    ASSERT_TRUE(4 == jsonObj.size("/o2"));
}


TEST(JsonTest, setNull)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json.setNull("/a");

    ASSERT_TRUE(json.isType("/a", json::Type::Null));
}

TEST(JsonTest, setEmpty)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json.setEmpty("/a");

    ASSERT_TRUE(json.isEmpty("/a"));
}

TEST(JsonTest, setObject)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json.setObject("/a");
    
    ASSERT_TRUE(json.isType("/a", json::Type::Object));
}

TEST(JsonTest, setAndGetObject)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    auto&& obj = json.setAndGetObject("/a");
    
    ASSERT_TRUE(json.isType("/a", json::Type::Object));

    ASSERT_TRUE(obj.IsObject());
}

TEST(JsonTest, setArray)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json.setArray("/a");
    
    ASSERT_TRUE(json.isType("/a", json::Type::Array));
}

TEST(JsonTest, setAndGetArray)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    auto&& arr = json.setAndGetArray("/a");
    
    ASSERT_TRUE(json.isType("/a", json::Type::Array));

    ASSERT_TRUE(arr.IsArray());
}

TEST(JsonTest, setType)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));

    json.setType("/i1", 1);
    ASSERT_TRUE(json.isType("/i1", json::Type::Int));
    ASSERT_TRUE(json.isType("/i1", json::Type::Int64));
    ASSERT_TRUE(json.isType("/i1", json::Type::Number));

    json.setType("/i2", 2147483648LL); // Exceeds int32
    ASSERT_TRUE(json.isType("/i2", json::Type::Int64));
    ASSERT_FALSE(json.isType("/i2", json::Type::Int));
    ASSERT_TRUE(json.isType("/i2", json::Type::Number));

    json.setType("/u1", static_cast<unsigned>(123456));
    ASSERT_TRUE(json.isType("/u1", json::Type::Uint));
    ASSERT_TRUE(json.isType("/u1", json::Type::Uint64));
    ASSERT_TRUE(json.isType("/u1", json::Type::Number));

    json.setType("/u2", static_cast<uint64_t>(1234567890123456789ULL));
    ASSERT_TRUE(json.isType("/u2", json::Type::Uint64));
    ASSERT_FALSE(json.isType("/u2", json::Type::Uint)); // Too large for uint32
    ASSERT_TRUE(json.isType("/u2", json::Type::Number));

    json.setType("/f", false);
    ASSERT_TRUE(json.isType("/f", json::Type::Boolean));

    json.setType("/float", 3.1416f);
    ASSERT_TRUE(json.isType("/float", json::Type::Float));
    ASSERT_TRUE(json.isType("/float", json::Type::Double));
    ASSERT_TRUE(json.isType("/float", json::Type::Number));

    json.setType("/double", 3.1416);
    ASSERT_TRUE(json.isType("/double", json::Type::Double));
    ASSERT_TRUE(json.isType("/double", json::Type::Number));

    json.setType("/hello", "World!");
    ASSERT_TRUE(json.isType("/hello", json::Type::String));

    json.setType("/string", std::string("std::string"));
    ASSERT_TRUE(json.isType("/string", json::Type::String));

    json.setType("/string_view", std::string_view("view"));
    ASSERT_TRUE(json.isType("/string_view", json::Type::String));

    const char* cstr = "cstring";
    json.setType("/cstring", cstr);
    ASSERT_TRUE(json.isType("/cstring", json::Type::String));

}

TEST(JsonTest, setTypeMany)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));
    
    const char* cstr = "cstring";

    json.setTypeMany({
        {"/i1", 1},
        {"/i2", 2147483648LL},
        {"/u1", static_cast<unsigned>(123456)},
        {"/u2", static_cast<uint64_t>(1234567890123456789ULL)},
        {"/f", false},
        {"/float", 3.1416f},
        {"/double", 3.1416},
        {"/string", std::string("std::string")},
        {"/string_view", std::string_view("std::string_view")},
        {"/cstring", cstr},
    });

    ASSERT_TRUE(json.isType("/i1", json::Type::Int));
    ASSERT_TRUE(json.isType("/i1", json::Type::Int64));
    ASSERT_TRUE(json.isType("/i1", json::Type::Number));

    ASSERT_TRUE(json.isType("/i2", json::Type::Int64));
    ASSERT_FALSE(json.isType("/i2", json::Type::Int));
    ASSERT_TRUE(json.isType("/i2", json::Type::Number));

    ASSERT_TRUE(json.isType("/u1", json::Type::Uint));
    ASSERT_TRUE(json.isType("/u1", json::Type::Uint64));
    ASSERT_TRUE(json.isType("/u1", json::Type::Number));

    ASSERT_TRUE(json.isType("/u2", json::Type::Uint64));
    ASSERT_FALSE(json.isType("/u2", json::Type::Uint)); // Too large for uint32
    ASSERT_TRUE(json.isType("/u2", json::Type::Number));

    ASSERT_TRUE(json.isType("/f", json::Type::Boolean));

    ASSERT_TRUE(json.isType("/float", json::Type::Float));
    ASSERT_TRUE(json.isType("/float", json::Type::Double));
    ASSERT_TRUE(json.isType("/float", json::Type::Number));

    ASSERT_TRUE(json.isType("/double", json::Type::Double));
    ASSERT_TRUE(json.isType("/double", json::Type::Number));

    ASSERT_TRUE(json.isType("/string", json::Type::String));
    ASSERT_TRUE(json.isType("/string_view", json::Type::String));
    ASSERT_TRUE(json.isType("/cstring", json::Type::String));
}

TEST(JsonTest, erase)
{
    std::optional<base::Error> err{ base::Error{} };

    json::Json json{jsonStr};
    err = json.getParseError();
    ASSERT_FALSE(base::isError(err));
    
    ASSERT_TRUE(json.erase("/t"));
    ASSERT_FALSE(json.erase("/t"));
    ASSERT_FALSE(json.erase("/non_existent"));
}

TEST(JsonTest, validate)
{
    std::optional<base::Error> err{ base::Error{} };

    // Valid schema
    const char* validSchemaStr = R"({
        "type": "object",
        "properties": {
            "name": { "type": "string" },
            "age": { "type": "integer" }
        },
        "required": ["name", "age"]
    })";

    // Valid JSON
    const char* validJsonStr = R"({
        "name": "John",
        "age": 30
    })";

    // Invalid JSON (missing 'age' field)
    const char* invalidJsonStr = R"({
        "name": "John"
    })";

    // Invalid JSON (misstype 'age' field)
    const char* invalidJsonStr2 = R"({
        "name": "John",
        "age": "123"
    })";

    // Create schema
    json::Json validSchema{validSchemaStr};
    err = validSchema.getParseError();
    ASSERT_FALSE(base::isError(err));

    // Try invalid json
    json::Json invalidJson{invalidJsonStr};
    err = invalidJson.getParseError();
    ASSERT_FALSE(base::isError(err));
    err = invalidJson.validate(validSchema);
    ASSERT_TRUE(base::isError(err));

    // Try invalid json 2
    json::Json invalidJson2{invalidJsonStr2};
    err = invalidJson2.getParseError();
    ASSERT_FALSE(base::isError(err));
    err = invalidJson2.validate(validSchema);
    ASSERT_TRUE(base::isError(err));

    // Try a valid json
    json::Json validJsonObj{validJsonStr};
    err = validJsonObj.getParseError();
    ASSERT_FALSE(base::isError(err));
    err = validJsonObj.validate(validSchema);
    ASSERT_FALSE(base::isError(err));
}
