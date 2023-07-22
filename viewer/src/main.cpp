#include <klast/klast.hpp>

#include <iostream>

constexpr int DEFAULT_WINDOW_WIDTH{ 1200 };
constexpr int DEFAULT_WINDOW_HEIGHT{ 675 };

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    try {
        klast::init();

        klast::Engine& engine{ klast::Engine::get() };
        klast::Engine::InitInfo initInfo{
            .windowInfo = {DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "Klast Rendering Engine"},
        };
        engine.init(initInfo);
        engine.run();

        klast::shutdown();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
