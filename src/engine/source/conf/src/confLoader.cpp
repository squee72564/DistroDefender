#include <fstream>
#include <sstream>

#include <conf/confLoader.hpp>

#include <base/logger.hpp>

namespace conf
{


json::Json ConfLoader::load() const
{
    const char* config_path = std::getenv("ENGINE_CONFIG_PATH");

    if (!config_path)
    {
        throw std::runtime_error(
            fmt::format(
                "ENGINE_CONFIG_PATH env var not set."
            )
        );
    }

    std::ifstream file{config_path};
    if (!file)
    {
        throw std::runtime_error(
            fmt::format(
                "Failed to open config file '{}'.",
                config_path 
            )
        );
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string jsonContent = buffer.str();

    json::Json config{};

    try
    {
        config = json::Json(jsonContent.c_str());
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            fmt::format(
                "Error while parsing configuration from {}: '{}'",
                config_path,
                e.what()
            )
        );
    }

    if (!config.isType("", json::Type::Object))
    {
        throw std::runtime_error(
            fmt::format(
                "Invalid configuration from {}: not an object",
                config_path 
            )
        );
    }

    return config;
}

} // namespace conf
