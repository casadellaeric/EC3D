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

    struct Info {
        bool validationLayers{};
        bool verticalSync{};
        std::vector<const char*> instanceExtensions{};
        std::vector<const char*> deviceExtensions{};
        Window::Info windowCreateInfo{};
    };

    Renderer() = default;
    explicit Renderer(const Renderer::Info& info);
    ~Renderer() noexcept;

    Renderer(const Renderer&)            = delete;
    Renderer(Renderer&&)                 = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&)      = default;

    void free();

    inline bool close_signalled() const { return m_window.close_signalled(); };
    inline void poll_events() { m_window.poll_events(); };
    inline const vk::Format get_presentable_image_format()
    {
        return m_device.get_swapchain_image_format();
    }

    inline GraphicsContext& create_graphics_context()
    {
        return m_device.create_graphics_context();
    };

private:

    void init(const Renderer::Info& info);
    void create_instance(const bool validationLayersEnabled,
                         const std::vector<const char*>& instExtensions);
    void create_debug_messenger();
    void create_device(const std::vector<const char*>& requestedExtensions,
                       bool verticalSyncEnabled);

private:

    Window m_window;

    vk::Instance m_instance;

    vk::DebugUtilsMessengerEXT m_debugMessenger;

    Device m_device;

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
