#include <conf/conf.hpp>

#include <unistd.h>

#include <fmt/format.h>


#include <base/logger.hpp>
#include <conf/keys.hpp>

namespace conf
{

using namespace internal;

Conf::Conf(std::shared_ptr<IConfLoader> confLoader)
    : config_{R"(null)"}
    , units_{}
    , confLoader_{confLoader}
{
    if (!confLoader_)
    {
        throw std::invalid_argument("The API loader cannot be null.");
    }

    // Register aviablable configuration units with Default Settings

    // Logging module
    addUnit<std::string>(key::LOG_LEVEL, "DD_LOG_LEVEL", "info");

    // KVDB module
    addUnit<std::string>(key::KVDB_PATH, "DD_KVDB_PATH", "/var/lib/distrodefender-server/engine/kvdb/");

    // Server module
    addUnit<std::string>(key::SERVER_API_SOCKET, "DD_SERVER_API_SOCKET", "/tmp/distro_defender_api.sock");
    addUnit<std::string>(key::SERVER_EVENT_SOCKET, "DD_SERVER_EVENT_SOCKET", "/tmp/distro_defender_event.sock");
};

void Conf::validate(const json::Json& config) const
{
    for (const auto& [key, value] : units_)
    {
        if (!config.exists(key))
        {
            continue; // The configuration is not set for this key, ignore it
        }

        const auto unitType = value->getType();
        switch (unitType)
        {
            case UnitConfType::INTEGER:
                if (config.isType(key, json::Type::Int) || config.isType(key, json::Type::Int64))
                {
                    continue;
                }
                throw std::runtime_error(
                    fmt::format("Invalid configuration type for key '{}'. Expected integer, got '{}'.",
                                key,
                                config.toStr(key).value_or("errorValue")));
            case UnitConfType::STRING:
                if (config.isType(key, json::Type::String))
                {
                    continue;
                }
                throw std::runtime_error(
                    fmt::format("Invalid configuration type for key '{}'. Expected string, got '{}'.",
                                key,
                                config.toStr(key).value_or("errorValue")));
            case UnitConfType::STRING_LIST:
                if (config.isType(key, json::Type::Array))
                {
                    auto jArr = config.getArray(key).value();

                    for (const auto& item : jArr)
                    {
                        if (!item.isType("", json::Type::String))
                        {
                            throw std::runtime_error(
                                fmt::format("Invalid configuration type for key '{}'. Expected string, got '{}'.",
                                            key,
                                            item.toStr()));
                        }
                    }
                    continue;
                }
                throw std::runtime_error(
                    fmt::format("Invalid configuration type for key '{}'. Expected array of strings, got '{}'.",
                                key,
                                config.toStr(key).value_or("errorValue")));
            case UnitConfType::BOOL:
                if (config.isType(key, json::Type::Boolean))
                {
                    continue;
                }
                throw std::runtime_error(
                    fmt::format("Invalid configuration type for key '{}'. Expected boolean, got '{}'.",
                                key,
                                config.toStr(key).value_or("errorValue")));
            default: throw std::logic_error(fmt::format("Invalid configuration type for key '{}'.", key));
        }
    }
}

void Conf::load()
{
    if (!config_.isType("", json::Type::Null))
    {
        throw std::logic_error("The configuration is already loaded.");
    }

    json::Json config = (*confLoader_)();
    validate(config);
    config_ = std::move(config);

    LOG_INFO("Config loaded:\n{}", config_.toStrPretty());
}

} // namespace conf
