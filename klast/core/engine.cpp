#include "pch.hpp"
#include "engine.hpp"
#include "utils/timer.hpp"

namespace klast
{

Engine::Engine() { }

Engine& Engine::get()
{
    static Engine instance;
    return instance;
}

void Engine::init(const Engine::InitInfo& initInfo)
{
    m_window.init(initInfo.windowInfo);
}

void Engine::run()
{
    Timer timer{};
    timer.reset();

    int frameCount{ 0 };
    float addedFrameTime{ 0.f };
    while (!m_window.close_signalled()) {
        m_window.poll_events();

        // FPS counting
        float dt{ timer.get_delta_time() };
        addedFrameTime += dt;
        ++frameCount;
        if (addedFrameTime >= 1.f) {
            KL_LOG_DEBUG("FPS: {}\nFrame time: {} ms", frameCount, addedFrameTime / frameCount);
            addedFrameTime = 0.f;
            frameCount     = 0;
        }
    }
}

}  // namespace klast
