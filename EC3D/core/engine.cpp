#include "pch.hpp"
#include "engine.hpp"
#include "misc/timer.hpp"

#include <vulkan/vulkan_structs.hpp>

namespace ec
{

Engine::Engine(const Engine::Info& info)
{
    try {
        init(info);
    }
    catch (const std::exception& e) {
        free();
        EC_LOG_CRITICAL("Unable to initialize the engine. Freeing engine resources. Reason: {}",
                        e.what());
        throw;
    };
}

Engine::~Engine() { }

void Engine::run()
{
    auto& context{ m_renderer.create_graphics_context() };
    create_test_renderpass(context);
    add_test_pipeline(context);
    // load_gltf_file("./models/Box.gltf");
    auto [vBuf, iBuf] = load_test_buffers();
    std::vector<vulkan::Buffer> vertexBuffers{ vBuf };

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
        testClearValue.color = { glm::sin(addedFrameTime) };
        context.set_clear_value(0, testClearValue);
        context.begin_rendering();
        context.bind_pipeline(0);
        context.bind_vertex_buffers(vertexBuffers);
        context.bind_index_buffer(iBuf);
        context.draw_indexed(iBuf.get_count());
        context.end_rendering();
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

void Engine::add_test_pipeline(vulkan::GraphicsContext& context)
{
    std::vector<vulkan::ShaderInfo> shaders{};
    shaders.resize(2);
    shaders[0] = {
        .filePath = "C:/EC3D/viewer/shaders/basic-vert.spv",
        .stage    = vk::ShaderStageFlagBits::eVertex,
    };
    shaders[1] = {
        .filePath = "C:/EC3D/viewer/shaders/basic-frag.spv",
        .stage    = vk::ShaderStageFlagBits::eFragment,
    };

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    std::vector<vulkan::VertexAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0] = { vk::Format::eR32G32B32Sfloat, 0, offsetof(Vertex, position) };
    attributeDescriptions[1] = { vk::Format::eR32G32B32Sfloat, 0, offsetof(Vertex, color) };

    vulkan::GraphicsPipelineInfo pipelineInfo{
        .shaders                  = shaders,
        .bindingStrides           = { sizeof(Vertex) },
        .attributeDescriptions    = attributeDescriptions,
        .blendEnableInAttachments = { VK_FALSE },
        .subpassIdx               = 0,
    };
    context.create_pipeline(pipelineInfo);
}

std::pair<vulkan::Buffer, vulkan::Buffer> Engine::load_test_buffers()
{
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    std::vector<Vertex> vb(3);
    vb[0] = {
        .position = {-0.5f, 0.5f, 1.f},
        .color    = {  1.f,  0.f, 0.f},
    };
    vb[1] = {
        .position = {0.5f, 0.5f, 1.f},
        .color    = { 0.f,  1.f, 0.f},
    };
    vb[2] = {
        .position = {0.f, -0.5f, 1.f},
        .color    = {0.f,   0.f, 1.f},
    };

    std::vector<uint16_t> ib(3);
    ib = { 0, 1, 2 };

    auto vertexBuffer{ m_renderer.create_buffer({
        .count    = static_cast<uint32_t>(vb.size()),
        .elemSize = static_cast<vk::DeviceSize>(sizeof(vb[0])),
        .usage    = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
    }) };

    auto indexBuffer{ m_renderer.create_buffer({
        .count    = static_cast<uint32_t>(ib.size()),
        .elemSize = static_cast<vk::DeviceSize>(sizeof(ib[0])),
        .usage    = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal,
        .indexType        = vk::IndexType::eUint16,
    }) };

    m_renderer.transfer_data(vertexBuffer, vertexBuffer.get_size(), vb.data());
    m_renderer.transfer_data(indexBuffer, indexBuffer.get_size(), ib.data());

    return std::make_pair(vertexBuffer, indexBuffer);
}

}  // namespace ec
