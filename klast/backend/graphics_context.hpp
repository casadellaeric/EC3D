#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include "swapchain.hpp"
#include "shader.hpp"

namespace klast::vulkan
{

// TODO: take into account if it's a transient attachment
struct AttachmentInfo {
    vk::Format format                    = vk::Format::eUndefined;
    vk::ImageAspectFlags aspect          = vk::ImageAspectFlagBits::eNone;
    vk::SampleCountFlagBits numSamples   = vk::SampleCountFlagBits::e1;
    vk::AttachmentLoadOp loadOp          = vk::AttachmentLoadOp::eDontCare;
    vk::AttachmentStoreOp storeOp        = vk::AttachmentStoreOp::eDontCare;
    vk::AttachmentLoadOp stencilLoadOp   = vk::AttachmentLoadOp::eDontCare;
    vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    vk::ImageLayout initialLayout        = vk::ImageLayout::eUndefined;
    vk::ImageLayout finalLayout          = vk::ImageLayout::eUndefined;
    VkClearValue clearValue              = {};
    // std::vector<vk::ImageLayout> subpassLayouts = { vk::ImageLayout::eUndefined };
};

struct SubpassInfo {
    std::vector<vk::AttachmentReference> inputAttachments{};
    std::vector<vk::AttachmentReference> colorAttachments{};
    std::vector<vk::AttachmentReference> resolveAttachments{};
    std::optional<vk::AttachmentReference> depthStencilAttachment{};
    std::vector<uint32_t> preserveAttachments{};
};

struct DependencyInfo {
    std::pair<uint32_t, uint32_t> subpasses{};
    std::pair<vk::PipelineStageFlags, vk::PipelineStageFlags> stages{};
    std::pair<vk::AccessFlags, vk::AccessFlags> access{};
};

struct RenderPassInfo {
    std::vector<AttachmentInfo> attachments;
    std::vector<SubpassInfo> subpasses;
    std::vector<DependencyInfo> dependencies;
};

class GraphicsContext
{
public:

    constexpr static uint32_t MAX_RENDERING_FRAMES{ 2 };

    struct Info {
        vk::Device device;
        vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;
        const Swapchain& swapchain;
        vk::Queue queue;
        uint32_t queueFamilyIndex;
    };

    GraphicsContext() = default;
    GraphicsContext(const GraphicsContext::Info& info);
    GraphicsContext(GraphicsContext&&) = default;
    ~GraphicsContext() noexcept        = default;

    GraphicsContext& operator=(GraphicsContext&&) = default;

    void free();

    void create_render_pass(const RenderPassInfo& info);
    void render();

    inline void set_clear_value(uint32_t attachmentIndex, VkClearValue& clearValue)
    {
        m_renderPassInfo.attachments[attachmentIndex].clearValue = clearValue;
    };

private:

    void create_framebuffer_image(vk::Format format,
                                  vk::ImageUsageFlags usage,
                                  vk::SampleCountFlagBits samples,
                                  vk::ImageLayout layout,
                                  vk::ImageAspectFlags aspect);
    void create_framebuffers();
    void create_pipeline(const vk::Pipeline pipeline);

    void create_frame_synchronization();
    void create_command_pool();
    void create_command_buffers();

    void record_commands(uint32_t frameIndex, uint32_t imageIndex);
    void submit_command_buffer(uint32_t frameIndex);
    void present(uint32_t frameIndex, uint32_t imageIndex);

private:

    vk::Device m_device;
    const vk::PhysicalDeviceMemoryProperties* m_deviceMemoryProperties;
    const Swapchain* m_swapchain;

    vk::RenderPass m_renderPass{};
    RenderPassInfo m_renderPassInfo{};

    std::vector<std::vector<Image>> m_framebufferImages{};  // {color, depth, ...} * swapchain img
    uint32_t m_swapchainAttachmentIndex{};
    std::vector<vk::Framebuffer> m_framebuffers{};

    vk::Queue m_graphicsQueue;
    uint32_t m_graphicsQueueIndex;

    // Per frame-in-flight
    uint32_t m_currentFrameIdx{};

    struct FrameData {
        vk::Semaphore imageAvailableSemaphore{};
        vk::Semaphore imageRenderedSemaphore{};
        vk::Fence imageRenderedFence{};

        vk::CommandPool commandPool{};
        std::vector<vk::CommandBuffer> commandBuffers{};
    };

    std::array<FrameData, MAX_RENDERING_FRAMES> m_frames{};

private:

    bool is_swapchain_compatible(const vk::AttachmentDescription& attachment);
};

}  // namespace klast::vulkan
