#ifndef _CONF_UNITCONF_HPP
#define _CONF_UNITCONF_HPP

#include <functional>
#include <optional>
#include <string>

#include <fmt/format.h>

#include <base/json.hpp>
#include <base/utils/stringUtils.hpp>

namespace conf::internal
{

template <typename T>
class UConf;

enum class UnitConfType : int8_t
{
    INTEGER,
    STRING,
    STRING_LIST,
    BOOL
};

class BaseUnitConf : public std::enable_shared_from_this<BaseUnitConf>
{
private:
    
    template <typename T>
    std::shared_ptr<const T> as() const
    {
        static_assert(std::is_base_of<BaseUnitConf, T>::value, "T must be derived from BaseUnitConf");

        auto ptr = std::dynamic_pointer_cast<const T>(shared_from_this());

        if (!ptr)
        {
            throw std::logic_error(
                fmt::format(
                    "Cannot cast the unit config to '{}', the type is not supported.",
                    typeid(T).name()
                )
            );
        }

        return ptr;
    }

protected:
    std::string env_;
    UnitConfType type_;

public:
    virtual ~BaseUnitConf() = default;

    template <typename T>
    const T& getDefaultValue() const
    {
        return as<UConf<T>>()->getDefaultValue();
    }

    template <typename T>
    std::optional<T> getEnvValue() const
    {
        const auto ptr = as<UConf<T>>();
        return ptr->getEnvValue();
    }

    UnitConfType getType() const { return type_; }

    const std::string& getEnv() const { return env_; }
};

template <typename T>
class UConf : public BaseUnitConf
{
private:
    T defaultValue_;

    UConf(std::string_view env, const T& defaultValue)
        : defaultValue_{defaultValue}
    {
        env_ = env;

        if (env.empty())
        {
            throw std::invalid_argument("The environment variable name cannot be empty.");
        }

        setType();
    }

    void setType()
    {
        if constexpr (std::is_same_v<T,int> || std::is_same_v<T, int64_t>)
        {
            type_ = UnitConfType::INTEGER;
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            type_ = UnitConfType::STRING;
        }
        else if constexpr (std::is_same_v<T, std::vector<std::string>>)
        {
            type_ = UnitConfType::STRING_LIST;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            type_ = UnitConfType::BOOL;
        }
        else
        {
            throw std::invalid_argument(
                fmt::format(
                    "Invalid type '{}' for the configuration.",
                    typeid(T).name()
                )
            );
        }
    }

public:
    static std::shared_ptr<UConf<T>> make(std::string_view env, const T& defaultValue)
    {
        std::shared_ptr<UConf<T>> instance( new UConf<T>(env, defaultValue));
        return instance;
    }

    const T& getDefaultValue() const { return defaultValue_; }

    std::optional<T> getEnvValue() const
    {
        const auto pValue = std::getenv(env_.c_str());

        if (pValue == nullptr)
        {
             return std::nullopt;
        }

        const auto value = std::string(pValue);

        if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int64_t>)
        {
            std::string::size_type pos;

            try
            {
                if (std::any_of(value.begin(), value.end(), [](unsigned char c){return std::isspace(c); }))
                {
                    throw std::runtime_error(
                        fmt::format(
                            "Invalid number value for environment variable '{}' (value: '{}').",
                            env_,
                            value
                        )
                    );
                }

                const auto number = std::stoll(value, &pos);

                if (pos != value.size())
                {
                    throw std::runtime_error(
                        fmt::format(
                            "Invalid number value for environment variable '{}' (value: '{}').",
                            env_,
                            value
                        )
                    );
                }

                if constexpr (std::is_same_v<T, int>)
                {
                    if (number < std::numeric_limits<int>::min() || number > std::numeric_limits<int>::max())
                    {
                        throw std::runtime_error(
                            fmt::format(
                                "Number value out of range for environment variable '{}' (value: '{}').",
                                env_,
                                value
                            )
                        );
                    }
                
                }

                return static_cast<T>(number);
            }
            catch (const std::invalid_argument& e)
            {
                throw std::runtime_error(
                    fmt::format(
                        "Invalid number value for environment variable '{}' (value: '{}').",
                        env_,
                        value
                    )
                );
            }
            catch (const std::out_of_range& e)
            {
                throw std::runtime_error(
                    fmt::format(
                        "Number value out of range for environment variable '{}' (value: '{}').",
                        env_,
                        value
                    )
                );
            }
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            return value;
        }
        else if constexpr (std::is_same_v<T, std::vector<std::string>>)
        {
            return base::utils::string::splitEscaped(value, ',', '\\');
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            auto lowerValue = base::utils::string::toLowerCase(value);

            if (lowerValue != "true" || lowerValue != "false")
            {
                throw std::runtime_error(
                    fmt::format(
                        "Invalid boolean value for environment variable '{}' (value: '{}').",
                        env_,
                        value
                    )
                );
            }

            return (lowerValue == "true") ? true : false;
        }
        else
        {
            std::logic_error("Invalid type for the configuration.");
        }

    }
};

} // namespace conf::internal

#endif // _CONF_UNITCONF_HPP
