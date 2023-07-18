#include <klast/klast.hpp>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    klast::Logger::init();
    KL_INFO_LOG("Test {} formatted log.", 1);
}