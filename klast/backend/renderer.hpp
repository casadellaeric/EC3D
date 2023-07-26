#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "window.hpp"
#include "device.hpp"

namespace klast::vulkan
{

class Renderer
{
public:

    struct CreateInfo {
        const Window::CreateInfo windowCreateInfo{};
        const std::string_view applicationName{};
        const uint32_t applicationVersion{};
        const bool validationLayersEnabled{ false };
        const bool verticalSyncEnabled{ false };
        const std::vector<const char*> instanceExtensions{};
        const std::vector<const char*> deviceExtensions{};
    };

    Renderer() = default;
    explicit Renderer(const Renderer::CreateInfo& createInfo);
    ~Renderer() noexcept;

    Renderer(const Renderer&)            = delete;
    Renderer(Renderer&&)                 = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&)      = default;

public:

    void free();
    inline bool close_signalled() { return m_window.close_signalled(); };
    inline void poll_events() { m_window.poll_events(); };

private:

    void init_vulkan(const Renderer::CreateInfo& createInfo);
    void create_instance(std::string_view applicationName,
                         uint32_t applicationVersion,
                         const std::vector<const char*>& instExtensions);
    void create_debug_messenger();
    void create_surface();
    void create_device(const std::vector<const char*>& requestedExtensions);
    void create_swapchain();
    void create_queues();

private:

    Window m_window;

    vk::Instance m_instance;

    vk::DebugUtilsMessengerEXT m_debugMessenger;

    Device m_device;

    vk::SwapchainKHR m_swapchain;

    // Misc. members
    bool m_validationLayersEnabled{};

private:  // Support objects and functions

    inline static vk::DynamicLoader m_dynamicLoader;  // To load all vulkan functions

    bool instance_supports_layers(const std::vector<const char*>& requiredLayerNames);
    bool instance_supports_extensions(const std::vector<const char*>& requiredExtNames);
};

// Support functions not bound to class Renderer (possibly moved out later)
VKAPI_ATTR vk::Bool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
               VkDebugUtilsMessageTypeFlagsEXT msgTypes,
               const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
               void* userData);

}  // namespace klast::vulkan
