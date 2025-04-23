#ifndef _CONF_CONF_HPP
#define _CONF_CONF_HPP

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <fmt/format.h>

#include <base/logger.hpp>
#include <base/json.hpp>

#include <conf/confLoader.hpp>
#include <conf/unitconf.hpp>

namespace conf
{

namespace
{
template <typename U>
std::string toStr(const U& value)
{
    if constexpr (std::is_same_v<U, std::vector<std::string>>)
    {
        std::string res{};

        for (const auto& item : value)
        {
            res += item + ",";
        }

        res.pop_back();
        return res;
    }

    if constexpr (std::is_same_v<U, bool>)
    {
        return (value) ? "true" : "false";
    }

    if constexpr (std::is_same_v<U, std::string>)
    {
        return value;
    }

    if constexpr (std::is_same_v<U, int> || std::is_same_v<U, int64_t>)
    {
        return fmt::format("{}", value);
    }

    throw std::runtime_error("The type is not supported.");
}
} // namespace

class Conf final
{
private:
    json::Json config_; ///< Configuration from the config file
    std::unordered_map<std::string, std::shared_ptr<internal::BaseUnitConf>> units_; ///< The configuration units
    std::shared_ptr<IConfLoader> confLoader_; ///< The config Loader


    void validate(const json::Json& config) const;

public:
    Conf() = delete;

    explicit Conf(std::shared_ptr<IConfLoader> confLoader);

    void load();

    template <typename T>
    void addUnit(std::string_view key, std::string_view env, const T& defaultValue)
    {
        if (!config_.isType("", json::Type::Null))
        {
            throw std::logic_error("The configuration is already loaded.");
        }

        if (key.empty())
        {
            throw std::logic_error("The key cannot be empty.");
        }

        for (const auto& [k, unit] : units_)
        {
            if (k == key)
            {
                throw std::invalid_argument(
                    fmt::format(
                        "The key '{}' is already registered.",
                        key
                    )
                );
            }

            if (unit->getEnv().compare(env) == 0)
            {
                throw std::invalid_argument(
                    fmt::format(
                        "The environment variable '{}' is already registered.",
                        env
                    )
                );
            }
        }

        units_[key.data()] = internal::UConf<T>::make(env, defaultValue);
    }

    template <typename T>
    T get(std::string_view key) const
    {


        if (units_.find(key.data()) == units_.end())
        {
            throw std::runtime_error(
                fmt::format(
                    "The key '{}' is not found in the configuration options.",
                    key
                )
            );
        }

        const auto& unit = units_.at(key.data());

        if (const auto envValue = unit->template getEnvValue<T>())
        {
            LOG_DEBUG(
                "Using configuration key '{}' from environment variable '{}': '{}'.",
                key,
                unit->getEnv(),
                toStr<T>(envValue.value())
            );

            return envValue.value();
        }

        auto getValueAndOrigin = [&](auto&& confGetter) -> std::pair<T, std::string>
        {
            auto opt = confGetter();
            if (opt.has_value())
            {
                return {static_cast<T>(opt.value()), "config file"};
            }

            return {unit->template getDefaultValue<T>(), "default"};
        };

        if constexpr (std::is_same_v<T, std::string>)
        {
            const auto& [value, org]
                = getValueAndOrigin([&] { return config_.getString(key.data()); });
                LOG_DEBUG("Using configuration key '{}' from {}: '{}'", key, org, value);
                return value;
        }
        else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int64_t>)
        {
            const auto& [value, org]
                = getValueAndOrigin([&] { return config_.getIntAsInt64(key.data()); });
                LOG_DEBUG("Using configuration key '{}' from {}: '{}'", key, org, value);
                return value;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            const auto& [value, org]
                = getValueAndOrigin([&] { return config_.getBool(key.data()); });
                LOG_DEBUG("Using configuration key '{}' from {}: '{}'", key, org, value);
                return value;
        }
        else if constexpr (std::is_same_v<T, std::vector<std::string>>)
        {
            auto jArr = config_.getArray(key.data());

            if (!jArr)
            {
                auto value = unit->template getDefaultValue<T>();
                LOG_DEBUG("Using configuration key '{}' from default: '{}'", key, toStr<T>(value));
                return value;
            }

            std::vector<std::string> result;
            for (const auto& item : jArr.value())
            {
                result.push_back(item.getString("").value_or("ERROR_VALUE"));
            }

            LOG_DEBUG("Using configuration key '{}' from config file: '{}", key, toStr<T>(result));
            return result;
        }

        throw std::runtime_error(
            "The type is not supported."
        );
    }
};

} // namespace conf

#endif // _CONF_CONF_HPP
