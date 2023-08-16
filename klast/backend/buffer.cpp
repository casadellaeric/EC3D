#include "pch.hpp"
#include "buffer.hpp"
#include "utils.hpp"

namespace klast::vulkan
{

Buffer::Buffer(vk::Device device, const Buffer::Info& info) :
  m_device{ &device }
{
    try {
        create(info);
    }
    catch (const std::exception& e) {
        KL_LOG_ERROR("Failed to create a buffer object. Reason: {}", e.what());
        free();
        throw;
    }
}

void Buffer::create(const Buffer::Info& info)
{
    vk::BufferCreateInfo bufferCreateInfo{
        .size        = info.size,
        .usage       = info.usage,
        .sharingMode = vk::SharingMode::eExclusive,
    };
    m_buffer = m_device->createBuffer(bufferCreateInfo);

    vk::MemoryRequirements bufferMemReq{ m_device->getBufferMemoryRequirements(m_buffer) };
    vk::MemoryAllocateInfo memAllocInfo{
        .allocationSize  = bufferMemReq.size,
        .memoryTypeIndex = find_memory_type_index(info.deviceMemoryProperties,
                                                  info.memoryProperties,
                                                  bufferMemReq.memoryTypeBits),
    };
    m_bufferMemory = m_device->allocateMemory(memAllocInfo);
    m_device->bindBufferMemory(m_buffer, m_bufferMemory, 0);
}

void Buffer::free()
{
    m_device->destroyBuffer(m_buffer);
}

}  // namespace klast::vulkan
