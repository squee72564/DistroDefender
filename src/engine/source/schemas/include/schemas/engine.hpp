#ifndef _SCHEMA_ENGINE_HPP
#define _SCHEMA_ENGINE_HPP

#include <optional>
#include <string>
#include <string_view>

namespace schemas::engine
{

enum ReturnStatus
{
    UNKNOWN = 0,
    OK = 1,
    ERROR = 2
};

struct GenericStatus_Response
{
    ReturnStatus status;
    std::optional<std::string> error;
};

struct GenericRequest
{
    std::optional<std::string> content;
};

constexpr std::string_view GENERIC_STATUS_RESPONSE_SCHEMA = R"({
    "type": "object",
    "properties": {
        "status": { "type": "integer", "enum": [0,1,2] },
        "error": { "type": "string" }
    },
    "required": ["status"]
})";

constexpr std::string_view GENERIC_REQUEST_SCHEMA = R"({
    "type": "object",
    "properties": {
        "content": { "type": "string" }
    },
    "required": []
})";

} // namespace schemas::engine

#endif // _SCHEMA_ENGINE_HPP
