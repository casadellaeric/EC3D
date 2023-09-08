#pragma once

#include "window.hpp"
#include "image.hpp"

namespace ec::vulkan
{

struct SwapchainImage {
    Image image;
};

class Swapchain
{
public:

    struct Info {
        vk::PhysicalDevice physDevice;
        vk::Device device;
        vk::SurfaceKHR surface;
        const std::tuple<int, int> framebufferSize;
        bool verticalSync;
    };

    Swapchain() = default;
    explicit Swapchain(const Swapchain::Info& info);
    Swapchain(Swapchain&&) = default;

    Swapchain& operator=(Swapchain&&) = default;

    void free(vk::Device device);
    void recreate();

    uint32_t acquire_next_image(vk::Semaphore& imageAvailableSemaphore) const;

    inline vk::SwapchainKHR get_handle() const { return m_swapchain; }
    const SwapchainImage& get_image(uint32_t index) const { return m_swapchainImages[index]; }
    size_t get_num_images() const { return m_swapchainImages.size(); }
    vk::Format get_format() const { return m_swapchainImageFormat.format; }
    const vk::Extent2D& get_extent() const { return m_swapchainExtent; }

private:

    void create(const Swapchain::Info& info);
    vk::SurfaceFormatKHR select_format(
        const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;
    vk::Extent2D select_extent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities,
                               const std::tuple<int, int> framebufferSize) const;

private:

    vk::Device m_device;

    vk::SwapchainKHR m_swapchain;

    vk::SurfaceFormatKHR m_swapchainImageFormat;
    vk::Extent2D m_swapchainExtent;

    std::vector<SwapchainImage> m_swapchainImages{};
};

}  // namespace ec::vulkan
