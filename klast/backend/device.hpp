#pragma once

#include <vulkan/vulkan.hpp>
#include "swapchain.hpp"
#include "graphics_context.hpp"

namespace klast::vulkan
{

struct QueueFamilyIndices {
    int graphics{ -1 };
    int present{ -1 };
    int transfer{ -1 };

    bool all_valid() { return graphics >= 0 && present >= 0 && transfer >= 0; }
};

class Device
{
public:

    struct Info {
        const std::vector<vk::PhysicalDevice>& availablePhysicalDevices;
        const std::vector<const char*>& extensionsToEnable;
        vk::SurfaceKHR surface;
        std::tuple<int, int> framebufferSize;
        bool verticalSync;
    };

    Device() = default;
    Device(const Device::Info& info);
    Device(Device&&) = default;
    ~Device() noexcept;

    Device& operator=(Device&&) = default;

    void free();
    void wait();

    const vk::PhysicalDevice& get_physical_handle() const { return m_physicalDevice; };
    const vk::Device& get_logical_handle() const { return m_logicalDevice; }
    const vk::Format get_swapchain_image_format() const { return m_swapchain.get_format(); }
    const vk::Extent2D get_swapchain_extent() const { return m_swapchain.get_extent(); }

    GraphicsContext& create_graphics_context();

private:

    void obtain_physical_device(const std::vector<vk::PhysicalDevice>& physDevices);
    void create_device(const std::vector<const char*>& extToEnable);
    void create_swapchain(std::tuple<int, int> framebufferSize, bool verticalSyncEnabled);

    bool physical_device_meets_requirements(const vk::PhysicalDevice& physDevice);
    QueueFamilyIndices obtain_queue_family_indices(vk::PhysicalDevice physDevice);

private:

    vk::PhysicalDevice m_physicalDevice;
    vk::PhysicalDeviceMemoryProperties m_memoryProperties;

    vk::Device m_logicalDevice;

    struct {
        vk::Queue graphics;
        vk::Queue present;
        vk::Queue transfer;
    } m_queues;
    QueueFamilyIndices m_queueFamilyIndices{};

    const vk::SurfaceKHR* m_surface;
    Swapchain m_swapchain;

    // For now just one to test
    std::optional<GraphicsContext> m_graphicsContext{};
};

// Support functions not bound to Device
bool device_supports_extensions(vk::PhysicalDevice physDevice,
                                const std::vector<const char*>& requiredExtNames);

}  // namespace klast::vulkan
