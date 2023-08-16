#pragma once

#include "backend/renderer.hpp"
#include "backend/utils.hpp"
#include "backend/graphics_context.hpp"

namespace klast
{

class Engine
{
public:

    struct Info {
        bool validationLayers{};
        bool verticalSync{};
        Window::Info windowInfo{};
    };

    explicit Engine(const Engine::Info& info);
    ~Engine() noexcept;

    Engine(const Engine&)            = delete;
    Engine(Engine&&)                 = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&)      = delete;

public:

    void run();
    void free();

private:

    void init(const Engine::Info& info);
    void create_test_renderpass(vulkan::GraphicsContext& context);

private:

    vulkan::Renderer m_renderer{};
};

}  // namespace klast
