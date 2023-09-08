#include "pch.hpp"
#include "device.hpp"
#include "utils.hpp"
#include "graphics_context.hpp"

#include <set>

namespace ec::vulkan
{

Device::Device(const Device::Info& info) :
  m_surface(&info.surface)
{
    try {
        obtain_physical_device(info.availablePhysicalDevices);
        create_device(info.extensionsToEnable);
        create_transfer_command_pool();
        create_swapchain(info.framebufferSize, info.verticalSync);
    }
    catch (const std::exception& e) {
        EC_LOG_CRITICAL("Unable to initialize Device. Reason: {}", e.what());
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

// TODO: Implement a resource manager to return a handle instead of the buffer itself
Buffer Device::create_buffer(const Buffer::Info& info)
{
    std::vector<uint32_t> queueFamilies = { static_cast<uint32_t>(m_queueFamilyIndices.graphics),
                                            static_cast<uint32_t>(m_queueFamilyIndices.transfer) };
    return Buffer(m_logicalDevice, m_memoryProperties, queueFamilies, info);
}

void Device::transfer_to_buffer(Buffer& dstBuffer, vk::DeviceSize size, void* data)
{
    // If the destination buffer is not device local, should probably use buffer.copy_data instead
    EC_ASSERT(dstBuffer.m_bufferLocation & vk::MemoryPropertyFlagBits::eDeviceLocal);

    Buffer::Info stagingBufferInfo{
        .count    = dstBuffer.get_count(),
        .elemSize = dstBuffer.get_size() / dstBuffer.get_count(),
        .usage    = vk::BufferUsageFlagBits::eTransferSrc,
        .memoryProperties
        = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
    };
    std::vector<uint32_t> queueFamilies = { static_cast<uint32_t>(m_queueFamilyIndices.transfer) };
    Buffer stagingBuffer(m_logicalDevice, m_memoryProperties, queueFamilies, stagingBufferInfo);
    stagingBuffer.copy_data(0, size, data);

    // TODO: Move all this into a function
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool        = m_transferCommandPool,
        .level              = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1,
    };
    vk::CommandBuffer transferCommandBuffer{ m_logicalDevice.allocateCommandBuffers(
        allocateInfo)[0] };

    vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
    };
    transferCommandBuffer.begin(beginInfo);

    vk::BufferCopy bufferCopyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size      = size,
    };
    transferCommandBuffer.copyBuffer(stagingBuffer.get_handle(),
                                     dstBuffer.get_handle(),
                                     1,
                                     &bufferCopyRegion);

    transferCommandBuffer.end();
    vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers    = &transferCommandBuffer,
    };
    m_queues.transfer.submit(submitInfo);

    m_queues.transfer.waitIdle();  // TODO: Remove this and use a semaphore

    m_logicalDevice.freeCommandBuffers(m_transferCommandPool, 1, &transferCommandBuffer);
    stagingBuffer.free();
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

void Device::create_transfer_command_pool()
{
    vk::CommandPoolCreateInfo commandPoolCreateInfo{
        .flags            = vk::CommandPoolCreateFlagBits::eTransient,
        .queueFamilyIndex = static_cast<uint32_t>(m_queueFamilyIndices.transfer),
    };
    m_transferCommandPool = m_logicalDevice.createCommandPool(commandPoolCreateInfo);
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
    bool foundDedicatedTransferQueue{};
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

        // Find dedicated transfer queue
        if ((queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
            && !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)) {
            queueFamilyIndices.transfer = i;
            foundDedicatedTransferQueue = true;
        }

        if (queueFamilyIndices.all_valid()) {
            break;
        }
    }
    if (!foundDedicatedTransferQueue) {
        EC_LOG_DEBUG(
            "Failed to find a dedicated transfer queue in a device! Using graphics queue.");
        queueFamilyIndices.transfer = queueFamilyIndices.graphics;
    }
    return queueFamilyIndices;
}

bool device_supports_extensions(vk::PhysicalDevice physDevice,
                                const std::vector<const char*>& requiredExtNames)
{
    const auto devExtProperties{ physDevice.enumerateDeviceExtensionProperties() };

#ifdef EC_LOGGING_ENABLED
    const auto deviceProps{ physDevice.getProperties() };
    std::string extMsg{ std::format("\nDevice {} supported extensions: \n",
                                    deviceProps.deviceName.data()) };
    for (auto& ext : devExtProperties) {
        extMsg.append(std::format("\t{}\n", ext.extensionName.data()));
    }
    EC_LOG_DEBUG("{}", extMsg);
#endif

    std::vector<const char*> devExtNames;
    std::ranges::transform(devExtProperties,
                           std::back_inserter(devExtNames),
                           [](const vk::ExtensionProperties& extProperty)
                           { return extProperty.extensionName.data(); });

    return contains_all_names(devExtNames, requiredExtNames);
}

}  // namespace ec::vulkan
