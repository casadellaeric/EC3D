#include "pch.hpp"
#include "engine.hpp"
#include "misc/timer.hpp"
#include <vulkan/vulkan_structs.hpp>

namespace klast
{

Engine::Engine(const Engine::Info& info)
{
    try {
        init(info);
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
    auto& context{ m_renderer.create_graphics_context() };
    create_test_renderpass(context);

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

        // Rendering loop
        VkClearValue testClearValue{};
        testClearValue.color = { glm::sin((float)frameCount / 144.f) };
        context.set_clear_value(0, testClearValue);
        context.render();
    }
}

void Engine::free()
{
    m_renderer.free();
}

void Engine::init(const Engine::Info& info)
{
    vulkan::Renderer::Info rendererInfo{
        .validationLayers   = info.validationLayers,
        .verticalSync       = info.verticalSync,
        .instanceExtensions = {},
        .deviceExtensions   = {},
        .windowCreateInfo   = info.windowInfo,
    };
    rendererInfo.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    m_renderer = vulkan::Renderer{ rendererInfo };
}

void Engine::create_test_renderpass(vulkan::GraphicsContext& context)
{
    vulkan::AttachmentInfo swapchainColorAttachment{
        // idx 0
        .format        = m_renderer.get_presentable_image_format(),
        .aspect        = vk::ImageAspectFlagBits::eColor,
        .numSamples    = vk::SampleCountFlagBits::e1,
        .loadOp        = vk::AttachmentLoadOp::eClear,
        .storeOp       = vk::AttachmentStoreOp::eStore,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout   = vk::ImageLayout::ePresentSrcKHR,
        .clearValue    = VkClearValue{ .color = { 0.4f, 0.3f, 0.2f, 1.f }, },
    };

    vk::AttachmentReference test = { 0, vk::ImageLayout::eColorAttachmentOptimal };
    vulkan::SubpassInfo firstSubpass{
        .colorAttachments = { { 0, vk::ImageLayout::eColorAttachmentOptimal } },
    };

    std::vector<vulkan::DependencyInfo> dependencies(2);
    dependencies[0]           = {};
    dependencies[0].subpasses = { VK_SUBPASS_EXTERNAL, 0 };
    dependencies[0].stages    = { vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                  vk::PipelineStageFlagBits::eColorAttachmentOutput };
    dependencies[0].access
        = { vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite };

    dependencies[1]           = {};
    dependencies[1].subpasses = { 0, VK_SUBPASS_EXTERNAL };
    dependencies[1].stages    = { vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                  vk::PipelineStageFlagBits::eBottomOfPipe };
    dependencies[1].access
        = { vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eNone };

    vulkan::RenderPassInfo renderPassInfo{
        .attachments  = { swapchainColorAttachment },
        .subpasses    = { firstSubpass },
        .dependencies = dependencies,
    };

    context.create_render_pass(renderPassInfo);
}

}  // namespace klast
