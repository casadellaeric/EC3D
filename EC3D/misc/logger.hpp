#pragma once

// For now, only an stdout logger

namespace ec::log
{

constexpr const char* MACRO_LOGGER_NAME{ "Debug Logger" };

enum class Severity {
    debug = 0,
    info,
    warning,
    error,
    critical,
    off,
};

void register_macro_logger(log::Severity severity);
void macro_log(log::Severity msgSeverity, std::source_location location, std::string_view message);
void shutdown();

}  // namespace ec::log

#ifndef NDEBUG
    #define EC_LOGGING_ENABLED
#endif

#ifdef EC_LOGGING_ENABLED

    #define EC_LOG_CRITICAL(...)                            \
        ec::log::macro_log(ec::log::Severity::critical,     \
                           std::source_location::current(), \
                           std::format(__VA_ARGS__))

    #define EC_LOG_ERROR(...)                               \
        ec::log::macro_log(ec::log::Severity::error,        \
                           std::source_location::current(), \
                           std::format(__VA_ARGS__))

    #define EC_LOG_WARN(...)                                \
        ec::log::macro_log(ec::log::Severity::warning,      \
                           std::source_location::current(), \
                           std::format(__VA_ARGS__))

    #define EC_LOG_INFO(...)                                \
        ec::log::macro_log(ec::log::Severity::info,         \
                           std::source_location::current(), \
                           std::format(__VA_ARGS__))

    #define EC_LOG_DEBUG(...)                               \
        ec::log::macro_log(ec::log::Severity::debug,        \
                           std::source_location::current(), \
                           std::format(__VA_ARGS__))

#else

    #define EC_LOG_CRITICAL(...)
    #define EC_LOG_ERROR(...)
    #define EC_LOG_WARN(...)
    #define EC_LOG_INFO(...)
    #define EC_LOG_DEBUG(...)

#endif
