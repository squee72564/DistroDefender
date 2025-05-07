#ifndef _CONF_KEYS_HPP
#define _CONF_KEYS_HPP

#include <string_view>

namespace conf::key
{

// LOGGING
constexpr std::string_view LOG_LEVEL = "/engine/logger/level";

// KVDB
constexpr std::string_view KVDB_PATH = "/engine/kvdb/path";

// STORE
constexpr std::string_view STORE_PATH = "/engine/store/path";

// SERVER
constexpr std::string_view SERVER_API_SOCKET = "/engine/server/api_socket";
constexpr std::string_view SERVER_EVENT_SOCKET = "/engine/server/event_socket";

// ARCHIVER
constexpr std::string_view ARCHIVER_PATH = "/engine/archiver/path";
constexpr std::string_view ARCHIVER_ENABLED = "/engine/archiver/enabled";

} // namespace conf::key

#endif // _CONF_KEYS_HPP
