#include "pch.hpp"
#include "engine.hpp"
#include "utils/timer.hpp"

namespace klast
{

Engine::Engine(const Engine::CreateInfo& createInfo) :
{
    try {
        init(createInfo);
    }
    catch (const std::exception& e) {
        free();
        KL_LOG_CRITICAL("Unable to initialize the engine. Freeing engine resources. Reason: {}",
                        e.what());
        throw;
    };
}

Engine::~Engine() { }

void Engine::run()
{
    Timer timer{};
    timer.reset();

    int frameCount{ 0 };
    float addedFrameTime{ 0.f };
    while (!m_renderer.close_signalled()) {
        m_renderer.poll_events();

        // FPS counting
        float dt{ timer.get_delta_time() };
        addedFrameTime += dt;
        ++frameCount;
        if (addedFrameTime >= 1.f) {
            std::cout << std::format("FPS: {}, Frame time: {} ms\n\n",
                                     frameCount,
                                     addedFrameTime / frameCount);
            addedFrameTime = 0.f;
            frameCount     = 0;
        }
    }
}

void Engine::free()
{
    m_renderer.free();
}

void Engine::init(const Engine::CreateInfo& createInfo)
{
    vulkan::Renderer::CreateInfo rendererCreateInfo{
        .windowCreateInfo        = createInfo.windowCreateInfo,
        .applicationName         = createInfo.rendererInfo.applicationName,
        .applicationVersion      = createInfo.rendererInfo.applicationVersion,
        .validationLayersEnabled = createInfo.rendererInfo.validationLayersEnabled,
        .verticalSyncEnabled     = createInfo.rendererInfo.verticalSyncEnabled,
        .instanceExtensions      = {},
        .deviceExtensions        = { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
    };

    m_renderer = vulkan::Renderer{ rendererCreateInfo };
}

}  // namespace klast
