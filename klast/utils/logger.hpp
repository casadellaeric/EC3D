#pragma once

// -- For the log macros --
#include <format>
#include <source_location>
// ------------------------

#include <spdlog/spdlog.h>

namespace klast
{

enum class LogSeverity {
    klSeverityOff,
    klSeverityCritical,
    klSeverityError,
    klSeverityWarning,
    klSeverityInfo,
    klSeverityDebug,
};

// For now only one logger. Other loggers (e.g. to a file) will be added as needed

class Logger
{
public:

    Logger() = delete;

    static void init();

    static void set_severity(LogSeverity minSeverityFilter);

    static void log(LogSeverity msgSeverity,
                    std::source_location location,
                    std::string_view message);

private:

    static std::shared_ptr<spdlog::logger> m_logger;

    constexpr static spdlog::level::level_enum kl_severity_to_spdlog_level(LogSeverity severity);
};

}  // namespace klast

// Logger macros for easy message passing

#define KL_LOGGING_ENABLED

#ifdef KL_LOGGING_ENABLED
    #define KL_CRITICAL_LOG(...)                                   \
        klast::Logger::log(klast::LogSeverity::klSeverityCritical, \
                           std::source_location::current(),        \
                           std::format(__VA_ARGS__))
    #define KL_ERROR_LOG(...)                                   \
        klast::Logger::log(klast::LogSeverity::klSeverityError, \
                           std::source_location::current(),     \
                           std::format(__VA_ARGS__))
    #define KL_WARN_LOG(...)                                      \
        klast::Logger::log(klast::LogSeverity::klSeverityWarning, \
                           std::source_location::current(),       \
                           std::format(__VA_ARGS__))
    #define KL_INFO_LOG(...)                                   \
        klast::Logger::log(klast::LogSeverity::klSeverityInfo, \
                           std::source_location::current(),    \
                           std::format(__VA_ARGS__))
    #define KL_DEBUG_LOG(...)                                   \
        klast::Logger::log(klast::LogSeverity::klSeverityDebug, \
                           std::source_location::current(),     \
                           std::format(__VA_ARGS__))
#else
    #define KL_CRITICAL_LOG(...)
    #define KL_ERROR_LOG(...)
    #define KL_WARN_LOG(...)
    #define KL_INFO_LOG(...)
    #define KL_DEBUG_LOG(...)
#endif