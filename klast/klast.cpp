#include "pch.hpp"
#include "klast.hpp"

namespace klast
{

void init()
{
    log::register_macro_logger(log::Severity::klSeverityDebug);
}

void shutdown() { }

}  // namespace klast
