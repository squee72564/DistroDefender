#ifndef _LOGGER_HPP
#define _LOGGER_HPP

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>


namespace logger
{
/**
 * @brief Default path for the error log file.
 * The default path where error logs should be saved.
 */
constexpr auto STD_ERR_PATH {"/dev/stderr"};

/**
 * @brief Default path for the info log file.
 * The default path where info logs should be saved.
 */
constexpr auto STD_OUT_PATH {"/dev/stdout"};

// constexpr auto WAZUH_LOG_HEADER {"%D %T wazuh-engine[%P] %s:%# at %!(): %l: %v"};

/**
 * @brief Default log header format.
 * The default format used for log messages.
 */
constexpr auto DEFAULT_LOG_HEADER {"%Y-%m-%d %T.%e %P:%t %l: %v"};

/**
 * @brief Log header format for debug messages.
 * The format used for log messages with debug level.
 * It includes additional information such as source file, function, and line number.
 */
constexpr auto LOG_DEBUG_HEADER {"%Y-%m-%d %T.%e %P:%t %s:%# at %!(): %l: %v"};

/**
 * @brief Default log level.
 * Possible values: "trace", "debug", "info", "warning", "error", "critical", "off".
 */
constexpr auto DEFAULT_LOG_LEVEL {"info"};

/**
 * @brief Default number of dedicated threads.
 * 0 means no dedicated threads.
 */
constexpr auto DEFAULT_LOG_THREADS {0};

/**
 * @brief Default size of the log threads' queue.
 */
constexpr auto DEFAULT_LOG_THREADS_QUEUE_SIZE {8192};

/**
 * @brief Default flush interval for logs.
 * Value in milliseconds.
 */
constexpr auto DEFAULT_LOG_FLUSH_INTERVAL {1};

/**
 * @brief Enum class defining log levels
 *
 * This enum class represents different logging levels
 */
enum class LogLevel
{
    Trace,      /**< Trace Logging level. */
    Debug,      /**< Debug Logging level. */
    Info,       /**< Information Logging level. */
    Warn,       /**< Warning Logging level, */
    Err,        /**< Error Logging level. */
    Critical,   /**< Critical Logging level. */
    Off,        /**< Turn off Logging. */
    Invalid
};

constexpr static auto levelToStr(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Trace: return "trace";
        case LogLevel::Debug: return "debug";
        case LogLevel::Info: return "info";
        case LogLevel::Warn: return "warning";
        case LogLevel::Err: return "error";
        case LogLevel::Critical: return "critical";
        case LogLevel::Off: return "off";
        default: return "invalid";
    }
}

constexpr static auto strToLevel(std::string_view level)
{
    if (level == levelToStr(LogLevel::Trace))
    {
        return LogLevel::Trace;
    }
    if (level == levelToStr(LogLevel::Debug))
    {
        return LogLevel::Debug;
    }
    if (level == levelToStr(LogLevel::Info))
    {
        return LogLevel::Info;
    }
    if (level == levelToStr(LogLevel::Warn))
    {
        return LogLevel::Warn;
    }
    if (level == levelToStr(LogLevel::Err))
    {
        return LogLevel::Err;
    }
    if (level == levelToStr(LogLevel::Critical))
    {
        return LogLevel::Critical;
    }
    if (level == levelToStr(LogLevel::Off))
    {
        return LogLevel::Off;
    }
    throw std::invalid_argument(fmt::format("Invalid log level: '{}'", level));
}
/**
 * @brief Structure for logging configuration parameters
 */
struct LoggerConfig
{
    std::string                     filePath{STD_OUT_PATH};
    LogLevel level                  {LogLevel::Info};
    const uint32_t flushInterval    {DEFAULT_LOG_FLUSH_INTERVAL};
    const uint32_t dedicatedThreads {DEFAULT_LOG_THREADS};
    const uint32_t queueSize        {DEFAULT_LOG_THREADS_QUEUE_SIZE};
    bool                            truncate{false};
};

/**
 * @brief Retrieves the default logger.
 * @return std::shared_ptr<spdlog::logger> The default logger.
 */
std::shared_ptr<spdlog::logger> getDefaultLogger();

/**
 * @brief Sets the log level.
 * @param level The log level
 */
void setLevel(LogLevel level);

/**
 * @brief Retrieves the log level.
 * @return LogLevel The log level.
 */
LogLevel getLevel();

/**
 * @brief Starts logging with the given configuration.
 * @param cfg Logger configuration struct
 */
void start(const LoggerConfig& cfg);

/**
 * @brief Stops logging
 */
void stop();

void testInit(LogLevel level = LogLevel::Warn);

} // namespace logger

#define LOG_TRACE(msg, ...)                                                                                            \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::trace, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...)                                                                                            \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::debug, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...)                                                                                             \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::info, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...)                                                                                          \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::warn, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)                                                                                            \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::err, msg, ##__VA_ARGS__)
#define LOG_CRITICAL(msg, ...)                                                                                         \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, SPDLOG_FUNCTION}, spdlog::level::critical, msg, ##__VA_ARGS__)

#define LOG_TRACE_L(functionName, msg, ...)                                                                            \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, functionName}, spdlog::level::trace, msg, ##__VA_ARGS__)
#define LOG_DEBUG_L(functionName, msg, ...)                                                                            \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, functionName}, spdlog::level::debug, msg, ##__VA_ARGS__)
#define LOG_INFO_L(functionName, msg, ...)                                                                             \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, functionName}, spdlog::level::info, msg, ##__VA_ARGS__)
#define LOG_WARNING_L(functionName, msg, ...)                                                                          \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, functionName}, spdlog::level::warn, msg, ##__VA_ARGS__)
#define LOG_ERROR_L(functionName, msg, ...)                                                                            \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, functionName}, spdlog::level::err, msg, ##__VA_ARGS__)
#define LOG_CRITICAL_L(functionName, msg, ...)                                                                         \
    logger::getDefaultLogger()->log(                                                                                  \
        spdlog::source_loc {__FILE__, __LINE__, functionName}, spdlog::level::critical, msg, ##__VA_ARGS__)

#endif // _LOGGER_HPP
