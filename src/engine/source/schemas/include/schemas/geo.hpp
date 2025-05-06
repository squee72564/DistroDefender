#ifndef _SCHEMAS_GEO_HPP
#define _SCHEMAS_GEO_HPP

namespace schemas::geo
{

constexpr std::string_view DB_POST_REQUEST_SCHEMA {R"({
    "type": "object",
    "required": ["path", "type"],
    "properties": {
        "path": { "type": "string" },
        "type": { "type": "string" }
    }
})"};

constexpr std::string_view DB_DELETE_REQUEST_SCHEMA {R"({
    "type": "object",
    "required": ["path"],
    "properties": {
        "path": { "type": "string" }
    }
})"};

constexpr std::string_view DB_LIST_REQUEST_SCHEMA {R"({})"};

constexpr std::string_view DB_REMOTE_UPSERT_REQUEST_SCHEMA {R"({
    "type": "object",
    "required": ["path", "type", "dbUrl", "hashUrl"],
    "properties": {
        "path": { "type": "string" },
        "type": { "type": "string" },
        "dbUrl": { "type": "string" },
        "hashUrl": { "type": "string" },
    }
})"};

inline const json::Json& getDBPostRequestSchema()
{
    static const json::Json schema(DB_POST_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getDBDeleteRequestSchema()
{
    static const json::Json schema(DB_DELETE_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getDBListRequestSchema()
{
    static const json::Json schema(DB_LIST_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getDBRemoteUpsertRequestSchema()
{
    static const json::Json schema(DB_REMOTE_UPSERT_REQUEST_SCHEMA.data());
    return schema;
}

} // namespace schemas::geo

#endif // _SCHEMAS_GEO_HPP
