#include "pch.hpp"
#include "logger.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace klast::log
{

constexpr spdlog::level::level_enum kl_severity_to_spdlog_level(const log::Severity severity);

std::shared_ptr<spdlog::logger> g_macroLogger;

void register_macro_logger(log::Severity severity)
{
    g_macroLogger = spdlog::stdout_color_mt(MACRO_LOGGER_NAME);
    spdlog::set_level(kl_severity_to_spdlog_level(severity));
}

void macro_log(log::Severity msgSeverity, std::source_location location, std::string_view message)
{
    g_macroLogger->log(
        { location.file_name(), static_cast<int>(location.line()), location.function_name() },
        kl_severity_to_spdlog_level(msgSeverity),
        message);
}

void shutdown()
{
    spdlog::shutdown();
}

constexpr spdlog::level::level_enum kl_severity_to_spdlog_level(const log::Severity severity)
{
    using enum log::Severity;
    switch (severity) {
        case klSeverityOff: return spdlog::level::off;
        case klSeverityCritical: return spdlog::level::critical;
        case klSeverityError: return spdlog::level::err;
        case klSeverityWarning: return spdlog::level::warn;
        case klSeverityInfo: return spdlog::level::info;
        case klSeverityDebug: [[fallthrough]];
        default: return spdlog::level::debug;
    }
}

}  // namespace klast::log
