#include "pch.hpp"
#include "device.hpp"
#include "utils.hpp"
#include <set>

namespace klast::vulkan
{

Device::Device(const Device::CreateInfo& createInfo)
{
    try {
        obtain_physical_device(createInfo.availablePhysicalDevices);
        create_device(createInfo.extensionsToEnable);
    }
    catch (const std::exception& e) {
        KL_LOG_CRITICAL("Unable to obtain a device that meets specified requirements. Reason: {}",
                        e.what());
        free();
        throw;
    }
}

void Device::free()
{
    if (m_logicalDevice) {
        m_logicalDevice.destroy();
        m_logicalDevice = nullptr;
    }
}

void Device::obtain_physical_device(const std::vector<vk::PhysicalDevice>& physDevices)
{
    if (physDevices.size() == 0) {
        throw std::runtime_error("Unable to find a physical device in the current instance!");
    }

    auto selPhysDevice = std::ranges::find_if(physDevices, physical_device_meets_requirements);
    if (selPhysDevice == physDevices.end()) {
        throw std::runtime_error("Unable to find a physical device that meets the requirements!");
    }

    m_physicalDevice = *selPhysDevice;
}

void Device::create_device(const std::vector<const char*>& extToEnable)
{
    std::set<int> uniqueQueueFamilyIndices{
        m_queueFamilyIndices.graphics,
        m_queueFamilyIndices.present,
        m_queueFamilyIndices.transfer,
    };

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilyIndices.size());

    float queuePriorities[]{ 1.f };
    for (auto queueFamilyIndex : uniqueQueueFamilyIndices) {
        queueCreateInfos.push_back(
            vk::DeviceQueueCreateInfo{ .queueFamilyIndex = static_cast<uint32_t>(queueFamilyIndex),
                                       .queueCount       = 1,
                                       .pQueuePriorities = queuePriorities });
    };

    // TODO: Add features
    vk::PhysicalDeviceFeatures physDeviceFeatures{};
    vk::DeviceCreateInfo deviceCreateInfo{ .queueCreateInfoCount
                                           = static_cast<uint32_t>(queueCreateInfos.size()),
                                           .pQueueCreateInfos = queueCreateInfos.data(),
                                           .enabledExtensionCount
                                           = static_cast<uint32_t>(extToEnable.size()),
                                           .ppEnabledExtensionNames = extToEnable.data(),
                                           .pEnabledFeatures        = &physDeviceFeatures };

    m_logicalDevice = m_physicalDevice.createDevice(deviceCreateInfo);
}

QueueFamilyIndices obtain_queue_family_indices(vk::PhysicalDevice physDevice)
{
    const auto queueFamilyProperties{ physDevice.getQueueFamilyProperties() };
    QueueFamilyIndices queueFamilyIndices{};
    for (int i = 0; i < queueFamilyProperties.size(); ++i) {
        auto& queueFamily{ queueFamilyProperties[i] };
        if (queueFamily.queueCount == 0) {
            continue;
        }
        if (queueFamily.queueFlags | vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphics = i;
        }
        // TODO: physDevice.getSurfaceSupportKHR();
        if (queueFamily.queueFlags | vk::QueueFlagBits::eCompute) {
            queueFamilyIndices.present = i;
        }
    }
}
// TODO
return queueFamilyIndices;
}

bool device_supports_extensions(vk::PhysicalDevice physDevice,
                                const std::vector<const char*>& requiredExtNames)
{
    const auto devExtProperties{ physDevice.enumerateDeviceExtensionProperties() };

    std::vector<const char*> devExtNames;
    std::ranges::transform(devExtProperties,
                           std::back_inserter(devExtNames),
                           [](const vk::ExtensionProperties& extProperty)
                           { return extProperty.extensionName.data(); });

    return contains_all_names(devExtNames, requiredExtNames);
}

bool physical_device_meets_requirements(const vk::PhysicalDevice& physDevice)
{
    // auto physDeviceProperties{ physDevice.getProperties() };
    // auto physDeviceFeatures{ physDevice.getFeatures() };
    std::vector<const char*> extToEnable{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    if (!device_supports_extensions(physDevice, extToEnable)) {
        return false;
    }

    auto queueFamilyIndices{ obtain_queue_family_indices(physDevice) };
    if (!queueFamilyIndices.all_valid()) {
        return false;
    }

    return true;
}

}  // namespace klast::vulkan
