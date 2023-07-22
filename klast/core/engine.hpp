#pragma once

#include "backend/renderer.hpp"
#include "window.hpp"

namespace klast
{

class Engine
{
public:

    struct InitInfo {
        Window::InitInfo windowInfo;
    };

public:

    Engine(const Engine&) = delete;

    static Engine& get();

    void init(const Engine::InitInfo& initInfo);
    void run();

private:

    Engine();  // Singleton private constructor

private:

    vulkan::Renderer m_renderer;
    Window m_window;
};

}  // namespace klast
