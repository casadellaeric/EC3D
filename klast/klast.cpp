#include "pch.hpp"
#include "klast.hpp"

namespace klast
{

void init()
{
    std::cout << "Initializing Klast." << std::endl;
    log::register_macro_logger(log::Severity::debug);
}

void shutdown()
{
    std::cout << "Shutting down Klast." << std::endl;
}

}  // namespace klast
