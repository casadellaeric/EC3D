#include "pch.hpp"
#include "graphics_context.hpp"
#include "image.hpp"
#include "misc/timer.hpp"

namespace klast::vulkan
{

GraphicsContext::GraphicsContext(const GraphicsContext::Info& info) :
  m_device{ info.device },
  m_deviceMemoryProperties{ &info.deviceMemoryProperties },
  m_graphicsQueue{ info.queue },
  m_graphicsQueueIndex{ info.queueFamilyIndex },
  m_swapchain{ &info.swapchain },
  m_currentFrameIdx{ 0 }
{
}

void GraphicsContext::free()
{
    m_graphicsQueue.waitIdle();
    for (auto& frame : m_frames) {
        if (frame.imageAvailableSemaphore) {
            m_device.destroySemaphore(frame.imageAvailableSemaphore);
            frame.imageAvailableSemaphore = nullptr;
        }
        if (frame.imageRenderedSemaphore) {
            m_device.destroySemaphore(frame.imageRenderedSemaphore);
            frame.imageRenderedSemaphore = nullptr;
        }
        if (frame.imageRenderedFence) {
            m_device.destroyFence(frame.imageRenderedFence);
            frame.imageRenderedFence = nullptr;
        }
        for (auto& cBuffer : frame.commandBuffers) {
            m_device.freeCommandBuffers(frame.commandPool, cBuffer);
        }
        frame.commandBuffers.clear();
        if (frame.commandPool) {
            m_device.destroyCommandPool(frame.commandPool);
            frame.commandPool = nullptr;
        }
    };
    if (!m_framebuffers.empty()) {
        for (auto& framebuffer : m_framebuffers) {
            m_device.destroyFramebuffer(framebuffer);
        }
    }
    m_framebuffers.clear();
    if (!m_framebufferImages.empty()) {
        for (auto& swapchainFramebuffer : m_framebufferImages) {
            for (size_t j = 0; j < swapchainFramebuffer.size(); ++j) {
                if (j != m_swapchainAttachmentIndex) {
                    swapchainFramebuffer[j].free();
                }
            }
        }
    }
    m_framebufferImages.clear();
    if (m_renderPass) {
        m_device.destroyRenderPass(m_renderPass);
        m_renderPass = nullptr;
    }
}

void GraphicsContext::create_render_pass(const RenderPassInfo& info)
{
    if (m_renderPass) {
        throw std::runtime_error("Render pass already exists in this context!");
    }

    m_renderPassInfo = info;

    struct FramebufferImageInfo {
        vk::Format format{};
        vk::SampleCountFlagBits samples{};
        vk::ImageLayout initialLayout{};
        vk::ImageUsageFlags usage{};
        vk::ImageAspectFlags aspect{};
        bool isSwapchainImage{};
    };
    // Gather information for later
    std::vector<FramebufferImageInfo> framebufferImageInfos(info.attachments.size());

    std::vector<vk::AttachmentDescription> attachmentDescriptions(info.attachments.size());
    bool swapchainAttachmentFound{};
    for (size_t i = 0; i < info.attachments.size(); ++i) {
        auto& currentAttachment{ info.attachments[i] };
        attachmentDescriptions[i] = {
            .format         = currentAttachment.format,
            .samples        = currentAttachment.numSamples,
            .loadOp         = currentAttachment.loadOp,
            .storeOp        = currentAttachment.storeOp,
            .stencilLoadOp  = currentAttachment.stencilLoadOp,
            .stencilStoreOp = currentAttachment.stencilStoreOp,
            .initialLayout  = currentAttachment.initialLayout,
            .finalLayout    = currentAttachment.finalLayout,
        };

        if (currentAttachment.finalLayout == vk::ImageLayout::ePresentSrcKHR) {
            if (swapchainAttachmentFound) {
                throw std::runtime_error("Multiple attachments set to present!");
            }
            swapchainAttachmentFound                  = true;
            framebufferImageInfos[i].isSwapchainImage = true;
            m_swapchainAttachmentIndex                = static_cast<uint32_t>(i);
            if (!is_swapchain_compatible(attachmentDescriptions[i])) {
                throw std::runtime_error(
                    "Attachment set to present source is not compatible with the swapchain!");
            }
        } else {
            framebufferImageInfos[i].format        = currentAttachment.format;
            framebufferImageInfos[i].samples       = currentAttachment.numSamples;
            framebufferImageInfos[i].initialLayout = currentAttachment.initialLayout;
            framebufferImageInfos[i].aspect        = currentAttachment.aspect;
        }
    }
    if (!swapchainAttachmentFound) {
        throw std::runtime_error("Swapchain attachment not found when creating the renderpass!");
    }

    std::vector<vk::SubpassDescription> subpassDescriptions(info.subpasses.size());
    for (size_t i = 0; i < subpassDescriptions.size(); ++i) {
        subpassDescriptions[i] = {
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .inputAttachmentCount
            = static_cast<uint32_t>(info.subpasses[i].inputAttachments.size()),
            .pInputAttachments = info.subpasses[i].inputAttachments.data(),
            .colorAttachmentCount
            = static_cast<uint32_t>(info.subpasses[i].colorAttachments.size()),
            .pColorAttachments       = info.subpasses[i].colorAttachments.data(),
            .pResolveAttachments     = info.subpasses[i].resolveAttachments.empty()
                                           ? nullptr
                                           : info.subpasses[i].resolveAttachments.data(),
            .pDepthStencilAttachment = info.subpasses[i].depthStencilAttachment.has_value()
                                           ? &info.subpasses[i].depthStencilAttachment.value()
                                           : nullptr,
            .preserveAttachmentCount
            = static_cast<uint32_t>(info.subpasses[i].preserveAttachments.size()),
            .pPreserveAttachments = info.subpasses[i].preserveAttachments.data(),
        };

        // Set usage for later creation of framebuffer images
        for (size_t j = 0; j < info.subpasses[i].inputAttachments.size(); ++j) {
            framebufferImageInfos[info.subpasses[i].inputAttachments[j].attachment].usage
                |= vk::ImageUsageFlagBits::eInputAttachment;
        }
        for (size_t j = 0; j < info.subpasses[i].colorAttachments.size(); ++j) {
            framebufferImageInfos[info.subpasses[i].colorAttachments[j].attachment].usage
                |= vk::ImageUsageFlagBits::eColorAttachment;
        }
        if (info.subpasses[i].depthStencilAttachment.has_value()) {
            framebufferImageInfos[info.subpasses[i].depthStencilAttachment.value().attachment].usage
                |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        }
    }

    std::vector<vk::SubpassDependency> subpassDependencies(info.dependencies.size());
    for (size_t i = 0; i < subpassDependencies.size(); ++i) {
        subpassDependencies[i] = {
            .srcSubpass      = info.dependencies[i].subpasses.first,
            .dstSubpass      = info.dependencies[i].subpasses.second,
            .srcStageMask    = info.dependencies[i].stages.first,
            .dstStageMask    = info.dependencies[i].stages.second,
            .srcAccessMask   = info.dependencies[i].access.first,
            .dstAccessMask   = info.dependencies[i].access.second,
            .dependencyFlags = {},
        };
    }

    vk::RenderPassCreateInfo renderPassCreateInfo{
        .attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size()),
        .pAttachments    = attachmentDescriptions.data(),
        .subpassCount    = static_cast<uint32_t>(subpassDescriptions.size()),
        .pSubpasses      = subpassDescriptions.data(),
        .dependencyCount = static_cast<uint32_t>(subpassDependencies.size()),
        .pDependencies   = subpassDependencies.data(),
    };

    try {
        m_renderPass = m_device.createRenderPass(renderPassCreateInfo);
    }
    catch (const std::exception& e) {
        KL_LOG_ERROR("Failed to create a render pass: {}", e.what());
        free();
        throw;
    }

    m_framebufferImages.resize(m_swapchain->get_num_images());
    try {
        for (size_t i = 0; i < m_framebufferImages.size(); ++i) {
            for (auto& imgInfo : framebufferImageInfos) {
                if (imgInfo.isSwapchainImage) {
                    m_framebufferImages[i].push_back({});  // Dummy, swapchain image already exists
                    continue;
                }
                create_framebuffer_image(imgInfo.format,
                                         imgInfo.usage,
                                         imgInfo.samples,
                                         imgInfo.initialLayout,
                                         imgInfo.aspect);
            }
        }
        create_framebuffers();
    }
    catch (const std::exception& e) {
        KL_LOG_ERROR("Failed to create framebuffers: {}", e.what());
        free();
        throw;
    }

    try {
        create_frame_synchronization();
    }
    catch (const std::exception& e) {
        KL_LOG_ERROR("Failed to create frame synchronization (semaphores/fence): {}", e.what());
        free();
        throw;
    }

    try {
        create_command_pool();
        create_command_buffers();
    }
    catch (const std::exception& e) {
        KL_LOG_ERROR("Failed to allocate command buffers: {}", e.what());
        free();
        throw;
    }
}

void GraphicsContext::render()
{
    std::ignore = m_device.waitForFences(m_frames[m_currentFrameIdx].imageRenderedFence,
                                         VK_TRUE,
                                         std::numeric_limits<uint64_t>::max());
    m_device.resetFences(m_frames[m_currentFrameIdx].imageRenderedFence);

    // TODO: Resize swapchain and window if result is not VK_SUCCESS
    uint32_t imgIdx{ m_swapchain->acquire_next_image(
        m_frames[m_currentFrameIdx].imageAvailableSemaphore) };

    m_device.resetCommandPool(m_frames[m_currentFrameIdx].commandPool);

    record_commands(m_currentFrameIdx, imgIdx);
    submit_command_buffer(m_currentFrameIdx);
    present(m_currentFrameIdx, imgIdx);

    m_currentFrameIdx = (m_currentFrameIdx + 1) % MAX_RENDERING_FRAMES;
}

void GraphicsContext::create_framebuffer_image(vk::Format format,
                                               vk::ImageUsageFlags usage,
                                               vk::SampleCountFlagBits samples,
                                               vk::ImageLayout layout,
                                               vk::ImageAspectFlags aspect)
{
    vk::Extent2D swapchainExtent{ m_swapchain->get_extent() };
    Image::Info imageInfo{
        .width                  = swapchainExtent.width,
        .height                 = swapchainExtent.height,
        .format                 = format,
        .usage                  = usage,
        .samples                = samples,
        .layout                 = layout,
        .memoryProperties       = vk::MemoryPropertyFlagBits::eDeviceLocal,
        .deviceMemoryProperties = *m_deviceMemoryProperties,
    };

    for (size_t i = 0; i < m_framebufferImages.size(); ++i) {
        Image framebufferImage{ m_device, imageInfo };
        framebufferImage.create_view(format, aspect);
        m_framebufferImages[i].push_back(std::move(framebufferImage));
    }
}

void GraphicsContext::create_framebuffers()
{
    m_framebuffers.resize(m_swapchain->get_num_images());
    // i = image idx in swapchain
    // j = attachment idx in the framebuffer
    for (size_t i = 0; i < m_framebuffers.size(); ++i) {
        std::vector<vk::ImageView> attachments(m_framebufferImages[i].size());
        for (size_t j = 0; j < attachments.size(); ++j) {
            if (j == m_swapchainAttachmentIndex) {
                attachments[j]
                    = m_swapchain->get_image(static_cast<uint32_t>(i)).image.get_image_view();
            } else {
                attachments[j] = m_framebufferImages[i][j].get_image_view();
            }
        }

        vk::Extent2D swapchainExtent{ (*m_swapchain).get_extent() };
        vk::FramebufferCreateInfo framebufferInfo{
            .renderPass      = m_renderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments    = attachments.data(),
            .width           = swapchainExtent.width,
            .height          = swapchainExtent.height,
            .layers          = 1,
        };

        m_framebuffers[i] = m_device.createFramebuffer(framebufferInfo);
    }
}

void GraphicsContext::create_command_pool()
{
    // TODO: Create one per thread
    vk::CommandPoolCreateInfo commandPoolCreateInfo{
        .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_graphicsQueueIndex,
    };
    for (auto& frame : m_frames) {
        frame.commandPool = m_device.createCommandPool(commandPoolCreateInfo);
    };
}

void GraphicsContext::create_command_buffers()
{
    for (auto& frame : m_frames) {
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
            .commandPool        = frame.commandPool,
            .level              = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = 1,
        };
        frame.commandBuffers = m_device.allocateCommandBuffers(commandBufferAllocateInfo);
    }
}

void GraphicsContext::create_frame_synchronization()
{
    for (auto& frame : m_frames) {
        frame.imageAvailableSemaphore = m_device.createSemaphore({});
        frame.imageRenderedSemaphore  = m_device.createSemaphore({});
        frame.imageRenderedFence      = m_device.createFence(vk::FenceCreateInfo{
                 .flags = vk::FenceCreateFlagBits::eSignaled,
        });
    }
}

void GraphicsContext::record_commands(uint32_t frameIndex, uint32_t imageIndex)
{
    std::vector<VkClearValue> clearValues(m_renderPassInfo.attachments.size());
    for (size_t i = 0; i < clearValues.size(); ++i) {
        clearValues[i] = m_renderPassInfo.attachments[i].clearValue;
    };

    VkRenderPassBeginInfo renderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, .renderPass = m_renderPass,
        .framebuffer = m_framebuffers[imageIndex],
        .renderArea  = vk::Rect2D{.offset = { 0, 0 }, .extent = m_swapchain->get_extent(), },
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data(),
    };

    auto& commandBuffer{ m_frames[frameIndex].commandBuffers[0] };

    commandBuffer.begin(vk::CommandBufferBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    });

    // C-style because vk::ClearValue doesn't want to work for some reason
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (size_t i = 0; i < m_renderPassInfo.subpasses.size(); ++i) {
        if (i != 0) {
            commandBuffer.nextSubpass(vk::SubpassContents::eInline);
            // Subpass-specific commands
        }
    };

    commandBuffer.endRenderPass();
    commandBuffer.end();
}

void GraphicsContext::submit_command_buffer(uint32_t frameIndex)
{
    vk::SemaphoreSubmitInfo waitSemaphoreInfo{
        .semaphore = m_frames[frameIndex].imageAvailableSemaphore,
        .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
    };

    vk::SemaphoreSubmitInfo signalSemaphoreInfo{
        .semaphore = m_frames[frameIndex].imageRenderedSemaphore,
        .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
    };

    vk::CommandBufferSubmitInfo commandBufferInfo{
        .commandBuffer = m_frames[frameIndex].commandBuffers[0],
    };

    vk::SubmitInfo2 submitInfo{
        .waitSemaphoreInfoCount   = 1,
        .pWaitSemaphoreInfos      = &waitSemaphoreInfo,
        .commandBufferInfoCount   = 1,
        .pCommandBufferInfos      = &commandBufferInfo,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos    = &signalSemaphoreInfo,
    };
    m_graphicsQueue.submit2(submitInfo, m_frames[frameIndex].imageRenderedFence);
}

void GraphicsContext::present(uint32_t frameIndex, uint32_t imageIndex)
{
    vk::SwapchainKHR swapchain{ m_swapchain->get_handle() };
    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &m_frames[frameIndex].imageRenderedSemaphore,
        .swapchainCount     = 1,
        .pSwapchains        = &swapchain,
        .pImageIndices      = &imageIndex,
    };

    // TODO: Use return value to resize window
    std::ignore = m_graphicsQueue.presentKHR(presentInfo);
}

bool GraphicsContext::is_swapchain_compatible(const vk::AttachmentDescription& attachment)
{
    if (attachment.format != (*m_swapchain).get_format()) {
        return false;
    }
    return true;
}

}  // namespace klast::vulkan
