#include "pch.hpp"
#include "image.hpp"
#include "utils.hpp"

namespace ec::vulkan
{

Image::Image(const vk::Device device, const vk::Image image) :
  m_device{ device },
  m_image{ image }
{
}

Image::Image(const vk::Device device, const Image::Info& info) :
  m_device{ device }
{
    try {
        create(info);
    }
    catch (const std::exception& e) {
        EC_LOG_ERROR("Failed to create an image object. Reason: {}", e.what());
        free();
        throw;
    }
}

vk::ImageView Image::create_view(vk::Format format, vk::ImageAspectFlags aspectFlags)
{
    EC_ASSERT(m_image);
    vk::ImageViewCreateInfo imageViewCreateInfo{
        .image    = m_image,
        .viewType = vk::ImageViewType::e2D,
        .format   = format,
        .components{
                    .r = vk::ComponentSwizzle::eR,
                    .g = vk::ComponentSwizzle::eG,
                    .b = vk::ComponentSwizzle::eB,
                    .a = vk::ComponentSwizzle::eA,
                    },
        .subresourceRange{
                    .aspectMask     = aspectFlags,
                    .baseMipLevel   = 0,
                    .levelCount     = 1,
                    .baseArrayLayer = 0,
                    .layerCount     = 1,
                    },
    };
    try {
        m_imageView = m_device.createImageView(imageViewCreateInfo);
    }
    catch (const std::exception& e) {
        EC_LOG_ERROR("Failed to create ImageView: {}", e.what());
        throw;
    }
    return m_imageView;
}

void Image::delete_view()
{
    if (m_imageView) {
        m_device.destroyImageView(m_imageView);
        m_imageView = nullptr;
    }
}

void Image::free()
{
    delete_view();
    if (m_image) {
        m_device.destroyImage(m_image);
        m_image = nullptr;
    }
    if (m_imageMemory) {
        m_device.freeMemory(m_imageMemory);
        m_imageMemory = nullptr;
    }
}

void Image::create(const Image::Info& info)
{
    // TODO: Fix hard-coded parameters
    vk::ImageCreateInfo imageCreateInfo{
        .imageType = vk::ImageType::e2D,
        .format    = info.format,
        .extent{
                .width  = info.width,
                .height = info.height,
                .depth  = 1,
                },
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .samples       = info.samples,
        .tiling        = vk::ImageTiling::eOptimal,
        .usage         = info.usage,
        .sharingMode   = vk::SharingMode::eExclusive,
        .initialLayout = info.layout,
    };
    m_image = m_device.createImage(imageCreateInfo);

    // Allocate image resource
    vk::MemoryRequirements imageMemReq{ m_device.getImageMemoryRequirements(m_image) };
    vk::MemoryAllocateInfo memAllocInfo{
        .allocationSize  = imageMemReq.size,
        .memoryTypeIndex = find_memory_type_index(info.deviceMemoryProperties,
                                                  info.memoryProperties,
                                                  imageMemReq.memoryTypeBits),
    };
    m_imageMemory = m_device.allocateMemory(memAllocInfo);
    m_device.bindImageMemory(m_image, m_imageMemory, 0);
}

}  // namespace ec::vulkan
