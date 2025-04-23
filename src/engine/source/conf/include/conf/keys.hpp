#ifndef _CONF_KEYS_HPP
#define _CONF_KEYS_HPP

#include <string_view>

namespace conf::key
{

// LOGGING
constexpr std::string_view LOG_LEVEL = "/engine/logger/level";

// KVDB
constexpr std::string_view KVDB_PATH = "/engine/kvdb/path";

} // namespace conf::key

#endif // _CONF_KEYS_HPP
