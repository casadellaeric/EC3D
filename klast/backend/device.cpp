#include "pch.hpp"
#include "device.hpp"
#include "utils.hpp"
#include "graphics_context.hpp"

#include <set>

namespace klast::vulkan
{

Device::Device(const Device::Info& info) :
  m_surface(&info.surface)
{
    try {
        obtain_physical_device(info.availablePhysicalDevices);
        create_device(info.extensionsToEnable);
        create_swapchain(info.framebufferSize, info.verticalSync);
    }
    catch (const std::exception& e) {
        KL_LOG_CRITICAL("Unable to initialize Device. Reason: {}", e.what());
        free();
        throw;
    }
}

Device::~Device() noexcept { }

void Device::free()
{
    m_logicalDevice.waitIdle();
    if (m_graphicsContext) {
        m_graphicsContext->free();
    }
    m_swapchain.free(m_logicalDevice);
    if (m_logicalDevice) {
        m_logicalDevice.destroy();
        m_logicalDevice = nullptr;
    }
}

GraphicsContext& Device::create_graphics_context()
{
    GraphicsContext::Info contextInfo{
        .device                 = m_logicalDevice,
        .deviceMemoryProperties = m_memoryProperties,
        .swapchain              = m_swapchain,
        .queue                  = m_queues.graphics,
        .queueFamilyIndex       = static_cast<uint32_t>(m_queueFamilyIndices.graphics),
    };
    m_graphicsContext.emplace(contextInfo);

    return m_graphicsContext.value();
}

void Device::obtain_physical_device(const std::vector<vk::PhysicalDevice>& physDevices)
{
    if (physDevices.size() == 0) {
        throw std::runtime_error("Unable to find a physical device in the current instance!");
    }

    auto selPhysDevice = std::ranges::find_if(physDevices,
                                              [&](vk::PhysicalDevice pd)
                                              { return physical_device_meets_requirements(pd); });
    if (selPhysDevice == physDevices.end()) {
        throw std::runtime_error("Unable to find a physical device that meets the requirements!");
    }

    m_physicalDevice   = *selPhysDevice;
    m_memoryProperties = m_physicalDevice.getMemoryProperties();
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

    // Hard-coded extensions here
    std::vector<const char*> extensions{ extToEnable };

    vk::PhysicalDeviceSynchronization2Features sync2Features{
        .synchronization2 = true,
    };

    vk::DeviceCreateInfo deviceCreateInfo{ .pNext = &sync2Features,
                                           .queueCreateInfoCount
                                           = static_cast<uint32_t>(queueCreateInfos.size()),
                                           .pQueueCreateInfos = queueCreateInfos.data(),
                                           .enabledExtensionCount
                                           = static_cast<uint32_t>(extensions.size()),
                                           .ppEnabledExtensionNames = extensions.data(),
                                           .pEnabledFeatures        = &physDeviceFeatures };

    m_logicalDevice = m_physicalDevice.createDevice(deviceCreateInfo);

    m_queues.graphics = m_logicalDevice.getQueue(m_queueFamilyIndices.graphics, 0);
    m_queues.present  = m_logicalDevice.getQueue(m_queueFamilyIndices.present, 0);
    m_queues.transfer = m_logicalDevice.getQueue(m_queueFamilyIndices.transfer, 0);
}

void Device::create_swapchain(std::tuple<int, int> framebufferSize, bool verticalSyncEnabled)
{
    Swapchain::Info swapchainInfo{
        .physDevice      = m_physicalDevice,
        .device          = m_logicalDevice,
        .surface         = *m_surface,
        .framebufferSize = framebufferSize,
        .verticalSync    = verticalSyncEnabled,
    };

    m_swapchain = Swapchain(swapchainInfo);
}

bool Device::physical_device_meets_requirements(const vk::PhysicalDevice& physDevice)
{
    // auto physDeviceProperties{ physDevice.getProperties() };
    // auto physDeviceFeatures{ physDevice.getFeatures() };

    std::vector<const char*> extToEnable{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    if (!device_supports_extensions(physDevice, extToEnable)) {
        return false;
    }

    auto queueFamilyIndices{ obtain_queue_family_indices(physDevice) };
    if (!queueFamilyIndices.all_valid()) {  // TODO: Allow a selection of queue families
        return false;
    }

    m_queueFamilyIndices = queueFamilyIndices;
    return true;
}

QueueFamilyIndices Device::obtain_queue_family_indices(vk::PhysicalDevice physDevice)
{
    const auto queueFamilyProperties{ physDevice.getQueueFamilyProperties() };
    QueueFamilyIndices queueFamilyIndices{};
    for (int i = 0; i < queueFamilyProperties.size(); ++i) {
        auto& queueFamily{ queueFamilyProperties[i] };
        if (queueFamily.queueCount == 0) {
            continue;
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphics = i;
        }

        auto surfaceSupport = physDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *m_surface);
        if (surfaceSupport) {
            queueFamilyIndices.present = i;
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
            queueFamilyIndices.transfer = i;
        }

        // if (queueFamilyIndices.all_valid()) {
        //     break;
        // }
    }
    return queueFamilyIndices;
}

bool device_supports_extensions(vk::PhysicalDevice physDevice,
                                const std::vector<const char*>& requiredExtNames)
{
    const auto devExtProperties{ physDevice.enumerateDeviceExtensionProperties() };

#ifdef KL_LOGGING_ENABLED
    const auto deviceProps{ physDevice.getProperties() };
    std::string extMsg{ std::format("\nDevice {} supported extensions: \n",
                                    deviceProps.deviceName.data()) };
    for (auto& ext : devExtProperties) {
        extMsg.append(std::format("\t{}\n", ext.extensionName.data()));
    }
    KL_LOG_DEBUG("{}", extMsg);
#endif

    std::vector<const char*> devExtNames;
    std::ranges::transform(devExtProperties,
                           std::back_inserter(devExtNames),
                           [](const vk::ExtensionProperties& extProperty)
                           { return extProperty.extensionName.data(); });

    return contains_all_names(devExtNames, requiredExtNames);
}

}  // namespace klast::vulkan
