#ifndef _SCHEMAS_KVDB_HPP
#define _SCHEMAS_KVDB_HPP

namespace schemas::kvdb
{

// GET ENTRY FROM DB
constexpr std::string_view DB_GET_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name", "key"],
    "properties": {
        "name": { "type": "string" },
        "key": { "type": "string" }
    }
})";

// GET ENTRIES FILTERED FROM A DB
constexpr std::string_view DB_SEARCH_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name", "prefix"],
    "properties": {
        "name": { "type": "string" },
        "prefix": { "type": "string" },
        "page": { "type": "integer" },
        "records": { "type": "integer" }
    }
})";

// DELETE AN ENTRY FROM A DB
constexpr std::string_view DB_DELETE_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name", "key"],
    "properties": {
        "name": { "type": "string" },
        "key": { "type": "string" }
    }
})";

// INSERT A NEW ENTRY IN A DB
constexpr std::string_view DB_PUT_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name", "entry"],
    "properties": {
        "name": { "type": "string" },
        "entry": {
            "type": "object",
            "required": ["key", "value"],
            "properties": {
                "key": { "type": "string" },
                "value": {}
            }
        }
    }
})";

// LIST ALL DBS
constexpr std::string_view MANAGER_GET_REQUEST_SCHEMA = R"({
    "type": "object",
    "properties": {
        "must_be_loaded": { "type": "boolean" },
        "filter_by_name": { "type": "string" }
    }
})";

// CREATE A NEW DB
constexpr std::string_view MANAGER_POST_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name"],
    "properties": {
        "name": { "type": "string" },
        "path": { "type": "string" }
    }
})";

// DELETE A DB
constexpr std::string_view MANAGER_DELETE_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name"],
    "properties": {
        "name": { "type": "string" }
    }
})";

// GET ALL ENTRIES FROM A DB
constexpr std::string_view MANAGER_DUMP_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name"],
    "properties": {
        "name": { "type": "string" },
        "page": { "type": "integer" },
        "records": { "type": "integer" }
    }
})";

inline const json::Json& getDBGetRequestSchema()
{
    static const json::Json schema(DB_GET_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getDBSearchRequestSchema()
{
    static const json::Json schema(DB_SEARCH_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getDBDeleteRequestSchema()
{
    static const json::Json schema(DB_DELETE_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getDBPutRequestSchema()
{
    static const json::Json schema(DB_PUT_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getManagerGetRequestSchema()
{
    static const json::Json schema(MANAGER_GET_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getManagerPostRequestSchema()
{
    static const json::Json schema(MANAGER_POST_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getManagerDeleteRequestSchema()
{
    static const json::Json schema(MANAGER_DELETE_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getManagerDumpRequestSchema()
{
    static const json::Json schema(MANAGER_DUMP_REQUEST_SCHEMA.data());
    return schema;
}

} // namespace schemas::kvdb

#endif // _SCHEMAS_KVDB_HPP
