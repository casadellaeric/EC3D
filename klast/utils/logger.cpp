#include "logger.hpp"

#include "assertion.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace klast
{

std::shared_ptr<spdlog::logger> Logger::m_logger;

void Logger::init()
{
    try {
        m_logger = spdlog::stdout_color_mt("Klast Console Output");
    }
    catch (const spdlog::spdlog_ex& exception) {
        throw std::runtime_error(exception.what());
    }
    set_severity(LogSeverity::klSeverityDebug);
}

void Logger::set_severity(LogSeverity minSeverityFilter)
{
    m_logger->set_level(kl_severity_to_spdlog_level(minSeverityFilter));
}

void Logger::log(LogSeverity msgSeverity, std::source_location location, std::string_view message)
{
    m_logger->log(
        { location.file_name(), static_cast<int>(location.line()), location.function_name() },
        kl_severity_to_spdlog_level(msgSeverity),
        message);
}

constexpr spdlog::level::level_enum Logger::kl_severity_to_spdlog_level(LogSeverity severity)
{
    using enum LogSeverity;
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

}  // namespace klast