#pragma once

#include <vulkan/vulkan.hpp>

namespace klast::vulkan
{

class Device
{
public:

    struct CreateInfo {
        const std::vector<vk::PhysicalDevice>& availablePhysicalDevices;
        const std::vector<const char*>& extensionsToEnable;
    };

    Device() = default;
    Device(const Device::CreateInfo& createInfo);
    ~Device() noexcept;

public:

    vk::PhysicalDevice& get_physical_handle() { return m_physicalDevice; };
    vk::Device& get_logical_handle() { return m_logicalDevice; }

    void free();
    void wait();

private:

    vk::PhysicalDevice m_physicalDevice;
    vk::Device m_logicalDevice;

    struct {
        vk::Queue graphics;
        vk::Queue present;
        vk::Queue transfer;
    } m_queues;

    QueueFamilyIndices m_queueFamilyIndices{};

private:

    void obtain_physical_device(const std::vector<vk::PhysicalDevice>& physDevices);
    void create_device(const std::vector<const char*>& extToEnable);
};

struct QueueFamilyIndices {
    int graphics{ -1 };
    int present{ -1 };
    int transfer{ -1 };

    bool all_valid() { return graphics >= 0 && present >= 0 && transfer >= 0; }
};

QueueFamilyIndices obtain_queue_family_indices(vk::PhysicalDevice physDevice);
// Support functions not bound to Device
bool device_supports_extensions(vk::PhysicalDevice physDevice,
                                const std::vector<const char*>& requiredExtNames);
bool physical_device_meets_requirements(const vk::PhysicalDevice& physDevice);

}  // namespace klast::vulkan
