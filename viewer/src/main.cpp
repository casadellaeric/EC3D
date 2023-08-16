#include <klast/klast.hpp>

#include <iostream>

constexpr int DEFAULT_WINDOW_WIDTH{ 1200 };
constexpr int DEFAULT_WINDOW_HEIGHT{ 675 };

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    try {
        klast::init();
        klast::Engine::Info engineInfo{
            .validationLayers = true,
            .verticalSync     = true,
            .windowInfo{
                        .width  = DEFAULT_WINDOW_WIDTH,
                        .height = DEFAULT_WINDOW_HEIGHT,
                        .name   = "Klast Rendering Engine",
                        }
        };
        klast::Engine engine{ engineInfo };
        engine.run();
        engine.free();

        klast::shutdown();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown exception was thrown!" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
