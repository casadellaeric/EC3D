#pragma once

#include <vulkan/vulkan.hpp>

namespace klast::vulkan
{

class Renderer
{
public:

private:

    vk::Instance m_instance;

    struct {
        vk::Device device;
        vk::PhysicalDevice physicalDevice;
    } m_device;
};

}  // namespace klast::vulkan
