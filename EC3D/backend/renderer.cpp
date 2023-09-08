#include "pch.hpp"
#include "renderer.hpp"
#include "utils.hpp"

#include <vma/vk_mem_alloc.h>
#include <ranges>
#include <set>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace ec::vulkan
{

Renderer::Renderer(const Renderer::Info& info) :
  m_window{ info.windowCreateInfo }
{
    try {
        init(info);
    }
    catch (const std::exception& e) {
        EC_LOG_CRITICAL("Unable to initialize Vulkan. Freeing renderer resources. Reason: {}",
                        e.what());
        free();
        throw;
    }
}

Renderer::~Renderer() noexcept { }

Buffer Renderer::create_buffer(const Buffer::Info& info)
{
    return m_device.create_buffer(info);
}

void Renderer::transfer_data(Buffer& dstBuffer, size_t size, void* data)
{
    m_device.transfer_to_buffer(dstBuffer, static_cast<vk::DeviceSize>(size), data);
}

void Renderer::init(const Renderer::Info& info)
{
    // Get instance independent function pointers with the default dynamic loader
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
        = m_dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    create_instance(info.validationLayers, info.instanceExtensions);
    if (info.validationLayers) {
        create_debug_messenger();
    }
    m_window.create_surface(m_instance);
    create_device(info.deviceExtensions, info.verticalSync);
}

void Renderer::free()
{
    m_device.free();
    if (m_window.get_surface()) {
        m_instance.destroySurfaceKHR(m_window.get_surface());
        m_window.delete_surface();
    }
    if (m_debugMessenger) {
        m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger);
        m_debugMessenger = nullptr;
    }
    if (m_instance) {
        m_instance.destroy();
        m_instance = nullptr;
    }
    m_window.free();
}

void Renderer::create_instance(bool validationLayersEnabled,
                               const std::vector<const char*>& requestedExtensions)
{
    vk::ApplicationInfo applicationInfo{
        .pApplicationName   = "",
        .applicationVersion = 0,
        .pEngineName        = "EC3D",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VK_MAKE_API_VERSION(0, 1, 3, 0),
    };

    std::vector<const char*> layersToEnable{};
    if (validationLayersEnabled) {
        layersToEnable.push_back("VK_LAYER_KHRONOS_validation");
        if (!instance_supports_layers(layersToEnable)) {
            throw std::runtime_error("Instance does not support required layers!");
        }
    }

    auto extToEnable{ m_window.get_required_extensions() };
    extToEnable.insert(extToEnable.end(), requestedExtensions.begin(), requestedExtensions.end());
    if (validationLayersEnabled) {
        extToEnable.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    if (!instance_supports_extensions(extToEnable)) {
        throw std::runtime_error("Instance does not support required extensions!");
    }

    vk::InstanceCreateInfo instanceCreateInfo{
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = static_cast<uint32_t>(layersToEnable.size()),
        .ppEnabledLayerNames     = layersToEnable.data(),
        .enabledExtensionCount   = static_cast<uint32_t>(extToEnable.size()),
        .ppEnabledExtensionNames = extToEnable.data(),
    };

    m_instance = vk::createInstance(instanceCreateInfo);

    // Update dynamic loader with instance functions
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);
}

void Renderer::create_debug_messenger()
{
    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        // vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                       | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                       | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = debug_callback,
        .pUserData       = nullptr,
    };

    m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugMessengerCreateInfo);
}

void Renderer::create_device(const std::vector<const char*>& requestedExtensions,
                             bool verticalSyncEnabled)
{
    std::vector<vk::PhysicalDevice> physDevices{ m_instance.enumeratePhysicalDevices() };

    Device::Info deviceInfo{
        .availablePhysicalDevices = physDevices,
        .extensionsToEnable       = requestedExtensions,
        .surface                  = m_window.get_surface(),
        .framebufferSize          = m_window.get_framebuffer_size(),
        .verticalSync             = verticalSyncEnabled,
    };

    m_device = Device(deviceInfo);

    // Update dynamic loader with device functions
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device.get_logical_handle());
}

bool Renderer::instance_supports_layers(const std::vector<const char*>& requiredLayerNames)
{
    const auto instLayerProperties{ vk::enumerateInstanceLayerProperties() };

#ifdef EC_LOGGING_ENABLED
    std::string layersMsg{ "\nSupported validation layers: \n" };
    for (auto& layer : instLayerProperties) {
        layersMsg.append(std::format("\t{}\n", layer.layerName.data()));
    }
    EC_LOG_DEBUG("{}", layersMsg);
#endif

    std::vector<const char*> instLayerNames;
    std::ranges::transform(instLayerProperties,
                           std::back_inserter(instLayerNames),
                           [](const vk::LayerProperties& layerProperty)
                           { return layerProperty.layerName.data(); });

    return contains_all_names(instLayerNames, requiredLayerNames);
}

bool Renderer::instance_supports_extensions(const std::vector<const char*>& requiredExtNames)
{
    const auto instExtProperties{ vk::enumerateInstanceExtensionProperties() };

#ifdef EC_LOGGING_ENABLED
    std::string extMsg{ "\nSupported instance extensions: \n" };
    for (auto& ext : instExtProperties) {
        extMsg.append(std::format("\t{}\n", ext.extensionName.data()));
    }
    EC_LOG_DEBUG("{}", extMsg);
#endif

    std::vector<const char*> instExtNames;
    std::ranges::transform(instExtProperties,
                           std::back_inserter(instExtNames),
                           [](const vk::ExtensionProperties& extProperty)
                           { return extProperty.extensionName.data(); });

    return contains_all_names(instExtNames, requiredExtNames);
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
               VkDebugUtilsMessageTypeFlagsEXT msgTypes,
               const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
               [[maybe_unused]] void* userData)
{
    auto debugMessage{ std::format(
        "\n-- DEBUG CALLBACK MESSAGE --"
        "\nMessage Severity: {}"
        "\nMessage Type: {}"
        "\nMessage: {}"
        "\nQueueLabelCount: {}",
        vk::to_string(vk::DebugUtilsMessageSeverityFlagBitsEXT(msgSeverity)),
        vk::to_string(vk::DebugUtilsMessageTypeFlagsEXT(msgTypes)),
        callbackData->pMessage,
        callbackData->queueLabelCount) };

    for (size_t i = 0; i < callbackData->queueLabelCount; ++i) {
        debugMessage.append(
            std::format("\n\tQueueLabel: {}", callbackData->pQueueLabels[i].pLabelName));
    }

    debugMessage.append(
        std::format("\nCommandBufferLabelCount: {}", callbackData->cmdBufLabelCount));

    for (size_t i = 0; i < callbackData->cmdBufLabelCount; ++i) {
        debugMessage.append(
            std::format("\n\tCommandBufferLabel: {}", callbackData->pCmdBufLabels[i].pLabelName));
    }

    debugMessage.append(std::format("\nObjectCount: {}", callbackData->objectCount));
    for (size_t i = 0; i < callbackData->objectCount; ++i) {
        const auto& object{ callbackData->pObjects[i] };
        debugMessage.append(std::format("\n\tObject: {}"
                                        "\n\tObjectType: {}"
                                        "\n\tObjectHandle: {}"
                                        "\n\tObjectName: {}"
                                        "\n",
                                        i,
                                        vk::to_string(vk::ObjectType(object.objectType)),
                                        object.objectHandle,
                                        object.pObjectName ? object.pObjectName : "-"));
    }

    switch (msgSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            EC_LOG_DEBUG("{}", debugMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: EC_LOG_INFO("{}", debugMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            EC_LOG_WARN("{}", debugMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: EC_LOG_ERROR("{}", debugMessage); break;
        default: EC_LOG_CRITICAL("{}", debugMessage); break;
    }

    return VK_FALSE;
}

}  // namespace ec::vulkan
