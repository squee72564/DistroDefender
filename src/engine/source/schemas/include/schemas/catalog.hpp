#ifndef _SCHEMAS_CATALOG_HPP
#define _SCHEMAS_CATALOG_HPP

#include <string_view>

#include <schemas/engine.hpp>

#include <base/json.hpp>

namespace schemas::catalog
{

enum ResourceType
{
    UNKNOWN = 0,
    DECODER,
    RULE,
    FILTER,
    OUTPUT,
    SCHEMA,
    COLLECTION,
    INTEGRATION
};

// POST A RESOURCE IN THE CATALOG
constexpr std::string_view RESOURCE_POST_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["type", "content", "namespaceid"],
    "properties": {
        "type": { "type": "string" },
        "content": { "type": "string" },
        "namespaceid": { "type": "string" }
    }
})";

// GET A RESOURCE FROM THE CATALOG
constexpr std::string_view RESOURCE_GET_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name", "namespaceid"],
    "properties": { 
        "name": { "type": "string" },
        "namespaceid": { "type": "string" }
    }
})";

constexpr std::string_view RESOURCE_GET_RESPONSE_SCHEMA = R"({
    "type": "object",
    "required": ["status", "error", "content"],
    "properties": {
        "status": { "type": "integer", "enum": [0,1,2] },
        "error": { "type": "string" },
        "content": { "type": "string" }
    }
})";

// PUT A RESOURCE IN THE CATALOG
constexpr std::string_view RESOURCE_PUT_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name", "content", "namespaceid"],
    "properties": { 
        "name": { "type": "string" },
        "content": { "type": "string" },
        "namespaceid": { "type": "string" }
    }
})";

// DELETE A RESOURCE FROM THE CATALOG
constexpr std::string_view RESOURCE_DELETE_REQUEST_SCHEMA = R"({
    "type": "object",
    "required": ["name", "namespaceid"],
    "properties": { 
        "name": { "type": "string" },
        "namespaceid": { "type": "string" }
    }
})";

// VALIDATE A RESOURCE FROM THE CATALOG
constexpr std::string_view RESOURCE_VALIDATE_SCHEMA = R"({
    "type": "object",
    "required": ["name", "content", "namespaceid"],
    "properties": { 
        "name": { "type": "string" },
        "content": { "type": "string" },
        "namespaceid": { "type": "string" }
    }
})";

// LIST ALL NAMESPACES FROM THE CATALOG
constexpr std::string_view RESOURCE_NAMESPACE_GET_REQUEST_SCHEMA = R"({
})";

constexpr std::string_view RESOURCE_NAMESPACE_GET_RESPONSE_SCHEMA = R"({
    "type": "object",
    "required": ["status", "namespaces"],
    "properties": {
        "status": { "type": "integer", "enum": [0,1,2] },
        "error": { "type": "string" },
        "namespaces": { "type": "array" }
    }
})";



inline const json::Json& getResourcePostRequestSchema()
{
    static const json::Json schema(RESOURCE_POST_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getResourceGetRequestSchema()
{
    static const json::Json schema(RESOURCE_GET_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getResourceGetResponseSchema()
{
    static const json::Json schema(RESOURCE_GET_RESPONSE_SCHEMA.data());
    return schema;
}

inline const json::Json& getResourcePutRequestSchema()
{
    static const json::Json schema(RESOURCE_PUT_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getResourceDeleteRequestSchema()
{
    static const json::Json schema(RESOURCE_DELETE_REQUEST_SCHEMA.data());
    return schema;
}

inline const json::Json& getResourceValidateSchema()
{
    static const json::Json schema(RESOURCE_VALIDATE_SCHEMA.data());
    return schema;
}

inline const json::Json& getResourceGetNamespaceRequestSchema()
{
    static const json::Json schema(RESOURCE_NAMESPACE_GET_REQUEST_SCHEMA.data());
    return schema;
}


inline const json::Json& getResourceGetNamespaceResponseSchema()
{
    static const json::Json schema(RESOURCE_NAMESPACE_GET_RESPONSE_SCHEMA.data());
    return schema;
}

} // namespace schemas::catalog

#endif // _SCHEMAS_CATALOG_HPP
