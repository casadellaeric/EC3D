#include "pch.hpp"
#include "renderer.hpp"
#include "utils.hpp"

#include <vma/vk_mem_alloc.h>
#include <ranges>
#include <set>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace klast::vulkan
{

Renderer::Renderer(const Renderer::CreateInfo& createInfo) :
  m_window{ createInfo.windowCreateInfo },
  m_validationLayersEnabled{ createInfo.validationLayersEnabled }
{
    try {
        init_vulkan(createInfo);
    }
    catch (const std::exception& e) {
        KL_LOG_CRITICAL("Unable to initialize Vulkan. Freeing renderer resources. Reason: {}",
                        e.what());
        free();
        throw;
    }
}

Renderer::~Renderer() noexcept { }

void Renderer::init_vulkan(const Renderer::CreateInfo& createInfo)
{
    // Get instance independent function pointers with the default dynamic loader
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
        = m_dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    create_instance(createInfo.applicationName,
                    createInfo.applicationVersion,
                    createInfo.instanceExtensions);
    if (m_validationLayersEnabled) {
        create_debug_messenger();
    }
    create_device(createInfo.deviceExtensions);
}

void Renderer::free()
{
    m_device.free();
    if (m_debugMessenger) {
        m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger);
        m_debugMessenger = nullptr;
    }
    if (m_instance) {
        m_instance.destroy();
        m_instance = nullptr;
    }
}

void Renderer::create_instance(std::string_view applicationName,
                               uint32_t applicationVersion,
                               const std::vector<const char*>& requestedExtensions)
{
    vk::ApplicationInfo applicationInfo{
        .pApplicationName   = applicationName.data(),
        .applicationVersion = applicationVersion,
        .pEngineName        = "Klast",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VK_MAKE_API_VERSION(0, 1, 3, 0),
    };

    std::vector<const char*> layersToEnable{};
    if (m_validationLayersEnabled) {
        layersToEnable.push_back("VK_LAYER_KHRONOS_validation");
        if (!instance_supports_layers(layersToEnable)) {
            throw std::runtime_error("Instance does not support required layers!");
        }
    }

    auto extToEnable{ m_window.get_required_extensions() };
    extToEnable.insert(extToEnable.end(), requestedExtensions.begin(), requestedExtensions.end());
    if (m_validationLayersEnabled) {
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
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                       | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                       | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = debug_callback,
        .pUserData       = nullptr,
    };

    m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugMessengerCreateInfo);
}

void Renderer::create_device(const std::vector<const char*>& requestedExtensions)
{
    std::vector<vk::PhysicalDevice> physDevices{ m_instance.enumeratePhysicalDevices() };

    Device::CreateInfo deviceCreateInfo{
        .availablePhysicalDevices = physDevices,
        .extensionsToEnable       = requestedExtensions,
    };

    m_device = Device(deviceCreateInfo);

    // Update dynamic loader with device functions
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device.get_logical_handle());
}

bool Renderer::instance_supports_layers(const std::vector<const char*>& requiredLayerNames)
{
    const auto instLayerProperties{ vk::enumerateInstanceLayerProperties() };

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
            KL_LOG_DEBUG("{}", debugMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: KL_LOG_INFO("{}", debugMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            KL_LOG_WARN("{}", debugMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: KL_LOG_ERROR("{}", debugMessage); break;
        default: KL_LOG_CRITICAL("{}", debugMessage); break;
    }

    return VK_FALSE;
}

}  // namespace klast::vulkan