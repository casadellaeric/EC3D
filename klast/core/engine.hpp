#pragma once

#include "backend/renderer.hpp"

namespace klast
{

class Engine
{
public:

    struct RendererCreateInfo {
        const Window::CreateInfo windowCreateInfo{};
        std::string_view applicationName{};
        uint32_t applicationVersion{};
        bool validationLayersEnabled{ false };
        bool verticalSyncEnabled{ false };
    };

    struct CreateInfo {
        Window::CreateInfo windowCreateInfo;
        Engine::RendererCreateInfo rendererInfo;
    };

    explicit Engine(const Engine::CreateInfo& createInfo);
    ~Engine() noexcept;

    Engine(const Engine&)            = delete;
    Engine(Engine&&)                 = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&)      = delete;

public:

    void run();
    void free();

private:

    void init(const Engine::CreateInfo& createInfo);

private:

    vulkan::Renderer m_renderer;
};

}  // namespace klast
