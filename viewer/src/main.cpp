#include <EC3D/ec3d.hpp>

#include <iostream>

constexpr int DEFAULT_WINDOW_WIDTH{ 1200 };
constexpr int DEFAULT_WINDOW_HEIGHT{ 675 };

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    try {
        ec::init();
        ec::Engine::Info engineInfo{
            .validationLayers = true,
            .verticalSync     = true,
            .windowInfo{
                        .width  = DEFAULT_WINDOW_WIDTH,
                        .height = DEFAULT_WINDOW_HEIGHT,
                        .name   = "EC3D Renderer",
                        }
        };
        ec::Engine engine{ engineInfo };
        engine.run();
        engine.free();

        ec::shutdown();
        system("pause");
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        system("pause");
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown exception was thrown!" << std::endl;
        system("pause");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
