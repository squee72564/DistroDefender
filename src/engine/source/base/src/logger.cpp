#include <base/logger.hpp>
#include <spdlog/pattern_formatter.h>

#include <iostream>

namespace logger
{
using LevelMap = std::unordered_map<LogLevel, spdlog::level::level_enum>;
using SpdlogMap = std::unordered_map<spdlog::level::level_enum, LogLevel>;
using LevelToStringMap = std::unordered_map<LogLevel, std::string_view>;
using StringToLevelMap = std::unordered_map<std::string_view, LogLevel>;

static const LevelMap LEVEL_MAP {
    {LogLevel::Trace, spdlog::level::trace},       /**< Trace Level mapping.  */
    {LogLevel::Debug, spdlog::level::debug},       /**< Debug Level mapping.  */
    {LogLevel::Info, spdlog::level::info},         /**< Info Level mapping.  */
    {LogLevel::Warn, spdlog::level::warn},         /**< Warn Level mapping.  */
    {LogLevel::Err, spdlog::level::err},           /**< Error Level mapping.  */
    {LogLevel::Critical, spdlog::level::critical}, /**< CriticalLevel mapping.  */
    {LogLevel::Off, spdlog::level::off}            /**< Off Level mapping.  */
};

static const SpdlogMap SPDLOG_MAP {
    {spdlog::level::trace, LogLevel::Trace},       /**< Trace Level mapping.  */
    {spdlog::level::debug, LogLevel::Debug},       /**< Debug Level mapping.  */
    {spdlog::level::info, LogLevel::Info},         /**< Info Level mapping.  */
    {spdlog::level::warn, LogLevel::Warn},         /**< Warn Level mapping.  */
    {spdlog::level::err,LogLevel::Err},            /**< Error Level mapping.  */
    {spdlog::level::critical, LogLevel::Critical}, /**< CriticalLevel mapping.  */
    {spdlog::level::off, LogLevel::Off}            /**< Off Level mapping.  */
};

static const LevelToStringMap LEVEL_TO_STRING {
    {LogLevel::Trace, "Trace"},
    {LogLevel::Debug, "Debug"},
    {LogLevel::Info, "Info"},
    {LogLevel::Warn, "Warining"},
    {LogLevel::Err, "Error"},
    {LogLevel::Critical, "Critical"},
    {LogLevel::Off, "OFF"}
};

static const StringToLevelMap STRING_TO_LEVEL {
    {"Trace", LogLevel::Trace},
    {"Debug", LogLevel::Debug},
    {"Info", LogLevel::Info},
    {"Warining", LogLevel::Warn},
    {"Error", LogLevel::Err},
    {"Critical", LogLevel::Critical},
    {"OFF", LogLevel::Off}
};

[[maybe_unused]] static auto logLevelToStr(LogLevel level)
{
    return LEVEL_TO_STRING.at(level); 
}


[[maybe_unused]] static auto strToLogLevel(std::string_view level)
{
    const auto it = STRING_TO_LEVEL.find(level);
    if (it == STRING_TO_LEVEL.end()) {
         throw std::invalid_argument(fmt::format("Invalid log level: '{}'", level));
    }

    return it->second;
}

class DefaultSink : public spdlog::sinks::sink
{
public:
    DefaultSink()
        : m_upFormatter(std::make_unique<spdlog::pattern_formatter>()) {}

    void log(const spdlog::details::log_msg& message) override
    {
        if (!should_log(message.level)) { return; }
        
        m_level = message.level;
        spdlog::memory_buf_t buff;
        m_upFormatter->format(message, buff);
        std::string formatted_message(buff.data(), buff.size());

        if (message.level >= spdlog::level::warn)
        {
            std::cerr << formatted_message;
        }
        else 
        {
            std::cout << formatted_message;
        }
    }

    void flush() override
    {
        if (m_level >= spdlog::level::warn)
        {
            std::cerr << std::flush;
        }
        else
        {
            std::cout << std::flush;
        }
    }

    void set_pattern(const std::string& pattern) override
    {
        m_upFormatter = std::make_unique<spdlog::pattern_formatter>(
            pattern,
            spdlog::pattern_time_type::local
        );
    }

    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override
    {
        m_upFormatter = std::move(sink_formatter);
    }

private:
    spdlog::level::level_enum m_level;
    std::unique_ptr<spdlog::formatter> m_upFormatter;
};

std::shared_ptr<spdlog::logger> getDefaultLogger()
{
    auto logger = spdlog::get("default");

    if (!logger)
    {
        throw std::runtime_error("The 'default' logger is not initialized!");
    }

    return logger;

}

LogLevel getLevel()
{
    auto spdLevel = getDefaultLogger()->level();
    return SPDLOG_MAP.at(spdLevel);
}

void setLevel(LogLevel level)
{
    auto logger = getDefaultLogger();
    logger->set_level(LEVEL_MAP.at(level));

    if (level <= LogLevel::Debug)
    {
        logger->set_pattern(LOG_DEBUG_HEADER);
    }
    else
    {
        logger->set_pattern(DEFAULT_LOG_HEADER);
    }
}

void start(const LoggerConfig& cfg)
{
    std::shared_ptr<spdlog::logger> logger;

    if (0 < cfg.dedicatedThreads)
    {
        spdlog::init_thread_pool(cfg.queueSize, cfg.dedicatedThreads);
    }

    if (cfg.filePath == STD_ERR_PATH || cfg.filePath == STD_OUT_PATH || cfg.filePath.empty())
    {
        auto sink = std::make_shared<DefaultSink>();

        logger = std::make_shared<spdlog::logger>("default", sink);
        spdlog::set_default_logger(logger);
    }
    else
    {
        logger = spdlog::basic_logger_mt("default", cfg.filePath, cfg.truncate);
    }

    setLevel(cfg.level);

    logger->flush_on(spdlog::level::trace);
}

void stop()
{
   spdlog::shutdown();
}

void testInit(LogLevel level)
{
    auto logger = spdlog::get("default");

    if (!logger)
    {
        LoggerConfig logConfig;
        logConfig.level = level;
        start(logConfig);
    }
}

} // namespace logger
