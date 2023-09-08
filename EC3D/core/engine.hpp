#pragma once

#include "backend/renderer.hpp"
#include "backend/utils.hpp"
#include "backend/graphics_context.hpp"

namespace ec
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
    void add_test_pipeline(vulkan::GraphicsContext& context);
    std::pair<vulkan::Buffer, vulkan::Buffer> load_test_buffers();

private:

    vulkan::Renderer m_renderer{};
};

}  // namespace ec
