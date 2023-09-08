#include "pch.hpp"
#include "swapchain.hpp"
#include "utils.hpp"
#include "misc/timer.hpp"

#include <limits>

namespace ec::vulkan
{

Swapchain::Swapchain(const Swapchain::Info& info) :
  m_device(info.device)
{
    try {
        create(info);
    }
    catch (const std::exception& e) {
        EC_LOG_CRITICAL("Unable to create a swapchain. Reason: {}", e.what());
        free(info.device);
        throw;
    }
}

void Swapchain::create(const Swapchain::Info& info)
{
    auto& surface{ info.surface };
    auto& physDevice{ info.physDevice };

    auto surfaceCapabilities{ physDevice.getSurfaceCapabilitiesKHR(surface) };
    uint32_t imageCount = (surfaceCapabilities.minImageCount + 1 > 3)
                              ? surfaceCapabilities.minImageCount + 1
                              : 3;  // We want triple buffering
    if (surfaceCapabilities.maxImageCount > 0 && surfaceCapabilities.maxImageCount < imageCount) {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    auto surfaceFormats{ physDevice.getSurfaceFormatsKHR(surface) };
    vk::SurfaceFormatKHR swapchainFormat{ select_format(surfaceFormats) };
    vk::Extent2D swapchainExtent{ select_extent(surfaceCapabilities, info.framebufferSize) };
    std::array<vk::CompositeAlphaFlagBitsKHR, 4> compositeAlphaValues{
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
        vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
        vk::CompositeAlphaFlagBitsKHR::eInherit
    };
    vk::CompositeAlphaFlagBitsKHR selectedCompositeAlpha{};
    for (auto& val : compositeAlphaValues) {
        if (surfaceCapabilities.supportedCompositeAlpha & val) {
            selectedCompositeAlpha = val;
            break;
        }
    }

    auto swapchainPresentMode{ vk::PresentModeKHR::eFifo };  // By default (should be available)
    // Prefer mailbox (uses newest image to display instead of first to arrive, not limited to
    // refresh rate of the monitor)
    if (!info.verticalSync) {
        auto surfacePresentModes{ physDevice.getSurfacePresentModesKHR(surface) };
        for (auto& presentMode : surfacePresentModes) {
            if (presentMode == vk::PresentModeKHR::eMailbox) {
                swapchainPresentMode = vk::PresentModeKHR::eMailbox;
                break;
            }
        }
    }

    vk::SwapchainCreateInfoKHR swapchainCreateInfo{
        .surface          = surface,
        .minImageCount    = imageCount,
        .imageFormat      = swapchainFormat.format,
        .imageColorSpace  = swapchainFormat.colorSpace,
        .imageExtent      = swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform     = surfaceCapabilities.currentTransform,
        .compositeAlpha   = selectedCompositeAlpha,
        .presentMode      = swapchainPresentMode,
        .clipped          = VK_TRUE,
        .oldSwapchain     = VK_NULL_HANDLE,
    };

    // TODO: Check other usages (transfer src/dst)
    m_swapchain = m_device.createSwapchainKHR(swapchainCreateInfo);

    m_swapchainImageFormat = swapchainFormat;
    m_swapchainExtent      = swapchainExtent;

    auto swapchainImages{ m_device.getSwapchainImagesKHR(m_swapchain) };
    m_swapchainImages.resize(swapchainImages.size());

    for (size_t i = 0; i < m_swapchainImages.size(); ++i) {
        Image swapchainImage{ m_device, swapchainImages[i] };
        swapchainImage.create_view(m_swapchainImageFormat.format, vk::ImageAspectFlagBits::eColor);

        m_swapchainImages[i] = {
            .image = std::move(swapchainImage),
        };
    }
}

void Swapchain::free(vk::Device device)
{
    for (auto& img : m_swapchainImages) {
        img.image.delete_view();
    }
    m_swapchainImages.clear();
    if (m_swapchain) {
        device.destroySwapchainKHR(m_swapchain);
        m_swapchain = nullptr;
    }
}

void Swapchain::recreate() { }

uint32_t Swapchain::acquire_next_image(vk::Semaphore& imageAvailableSemaphore) const
{
#undef max
    return m_device
        .acquireNextImageKHR(m_swapchain,
                             std::numeric_limits<uint64_t>::max(),
                             imageAvailableSemaphore)
        .value;
}

vk::SurfaceFormatKHR Swapchain::select_format(
    const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
{
    if (availableFormats.size() == 1 && (availableFormats[0].format == vk::Format::eUndefined)) {
        // All formats available
        return vk::SurfaceFormatKHR{ vk::Format::eR8G8B8A8Unorm,
                                     vk::ColorSpaceKHR::eSrgbNonlinear };
    }

    for (auto& format : availableFormats) {
        if ((format.format == vk::Format::eR8G8B8A8Unorm
             || format.format == vk::Format::eB8G8R8A8Unorm)
            && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }

    return availableFormats[0];
}

vk::Extent2D Swapchain::select_extent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
                                      const std::tuple<int, int> framebufferSize) const
{
#undef max
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>().max()) {
        return surfaceCapabilities.currentExtent;
    }

    auto [width, height] = framebufferSize;
    vk::Extent2D newExtent{};
    newExtent.width  = std::clamp(static_cast<uint32_t>(width),
                                 surfaceCapabilities.minImageExtent.width,
                                 surfaceCapabilities.maxImageExtent.width);
    newExtent.height = std::clamp(static_cast<uint32_t>(height),
                                  surfaceCapabilities.minImageExtent.height,
                                  surfaceCapabilities.maxImageExtent.height);
    return newExtent;
}

}  // namespace ec::vulkan
