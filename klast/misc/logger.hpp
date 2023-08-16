#pragma once

// For now, only an stdout logger

namespace klast::log
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

}  // namespace klast::log

#ifndef NDEBUG
    #define KL_LOGGING_ENABLED
#endif

#ifdef KL_LOGGING_ENABLED

    #define KL_LOG_CRITICAL(...)                               \
        klast::log::macro_log(klast::log::Severity::critical,  \
                              std::source_location::current(), \
                              std::format(__VA_ARGS__))
    #define KL_LOG_ERROR(...)                                  \
        klast::log::macro_log(klast::log::Severity::error,     \
                              std::source_location::current(), \
                              std::format(__VA_ARGS__))
    #define KL_LOG_WARN(...)                                   \
        klast::log::macro_log(klast::log::Severity::warning,   \
                              std::source_location::current(), \
                              std::format(__VA_ARGS__))
    #define KL_LOG_INFO(...)                                   \
        klast::log::macro_log(klast::log::Severity::info,      \
                              std::source_location::current(), \
                              std::format(__VA_ARGS__))
    #define KL_LOG_DEBUG(...)                                  \
        klast::log::macro_log(klast::log::Severity::debug,     \
                              std::source_location::current(), \
                              std::format(__VA_ARGS__))
#else

    #define KL_LOG_CRITICAL(...)
    #define KL_LOG_ERROR(...)
    #define KL_LOG_WARN(...)
    #define KL_LOG_INFO(...)
    #define KL_LOG_DEBUG(...)

#endif
