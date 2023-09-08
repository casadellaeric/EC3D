#include "pch.hpp"
#include "ec3d.hpp"

namespace ec
{

void init()
{
    std::cout << "Initializing EC3D." << std::endl;
    log::register_macro_logger(log::Severity::debug);
}

void shutdown()
{
    std::cout << "Shutting down EC3D." << std::endl;
}

}  // namespace ec
