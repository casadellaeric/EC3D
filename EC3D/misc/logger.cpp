#include "pch.hpp"
#include "logger.hpp"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace ec::log
{

constexpr spdlog::level::level_enum ec_severity_to_spdlog_level(const log::Severity severity);

std::shared_ptr<spdlog::logger> g_macroLogger;

void register_macro_logger(log::Severity severity)
{
    g_macroLogger = spdlog::stdout_color_mt(MACRO_LOGGER_NAME);
    spdlog::set_level(ec_severity_to_spdlog_level(severity));
}

void macro_log(log::Severity msgSeverity, std::source_location location, std::string_view message)
{
    g_macroLogger->log(
        { location.file_name(), static_cast<int>(location.line()), location.function_name() },
        ec_severity_to_spdlog_level(msgSeverity),
        message);
}

void shutdown()
{
    spdlog::shutdown();
}

constexpr spdlog::level::level_enum ec_severity_to_spdlog_level(const log::Severity severity)
{
    using enum log::Severity;
    switch (severity) {
        case off: return spdlog::level::off;
        case critical: return spdlog::level::critical;
        case error: return spdlog::level::err;
        case warning: return spdlog::level::warn;
        case info: return spdlog::level::info;
        case debug: [[fallthrough]];
        default: return spdlog::level::debug;
    }
}

}  // namespace ec::log
