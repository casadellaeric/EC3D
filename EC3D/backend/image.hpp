#pragma once

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

namespace ec::vulkan
{

class Image
{
public:

    struct Info {
        uint32_t width;
        uint32_t height;
        vk::Format format;
        vk::ImageUsageFlags usage;
        vk::SampleCountFlagBits samples;
        vk::ImageLayout layout;
        vk::MemoryPropertyFlags memoryProperties;
        const vk::PhysicalDeviceMemoryProperties& deviceMemoryProperties;
    };

    // This constructor assumes the image is already allocated (e.g. swapchain image)
    Image() = default;
    Image(const vk::Device device, const vk::Image image);
    Image(const vk::Device device, const Image::Info& info);
    Image(Image&&) = default;

    Image& operator=(const Image&) = default;
    Image& operator=(Image&&)      = default;

    vk::ImageView create_view(vk::Format format, vk::ImageAspectFlags aspectFlags);
    void delete_view();

    inline vk::Image get_image() const { return m_image; }
    inline vk::ImageView get_image_view() const { return m_imageView; }

    void free();

private:

    void create(const Image::Info& info);

private:

    vk::Device m_device;

    vk::Image m_image{};
    vk::DeviceMemory m_imageMemory{};

    vk::ImageView m_imageView{};
};

}  // namespace ec::vulkan
