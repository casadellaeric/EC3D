#include "pch.hpp"
#include "buffer.hpp"
#include "utils.hpp"

namespace ec::vulkan
{

Buffer::Buffer(vk::Device device,
               const vk::PhysicalDeviceMemoryProperties& deviceMemoryProperties,
               const std::vector<uint32_t>& queueFamilies,
               const Buffer::Info& info) :
  m_device{ device },
  m_deviceMemoryProperties{ deviceMemoryProperties }
{
    try {
        create(info, queueFamilies);
    }
    catch (const std::exception& e) {
        EC_LOG_ERROR("Failed to create a buffer object. Reason: {}", e.what());
        free();
        throw;
    }
}

void Buffer::create(const Buffer::Info& info, const std::vector<uint32_t>& queueFamilies)
{
    // TODO: Change sharing mode to exclusive when possible. Transfer ownership after submiting data
    // e.g. if the buffer is a static vertex buffer.
    vk::SharingMode bufferSharingMode{ queueFamilies.size() > 1 ? vk::SharingMode::eConcurrent
                                                                : vk::SharingMode::eExclusive };
    m_bufferCount    = info.count;
    m_bufferByteSize = info.elemSize * info.count;
    m_indexType      = info.indexType;

    vk::BufferCreateInfo bufferCreateInfo{
        .size                  = m_bufferByteSize,
        .usage                 = info.usage,
        .sharingMode           = bufferSharingMode,
        .queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size()),
        .pQueueFamilyIndices   = queueFamilies.data(),
    };
    m_buffer = m_device.createBuffer(bufferCreateInfo);

    vk::MemoryRequirements bufferMemReq{ m_device.getBufferMemoryRequirements(m_buffer) };
    vk::MemoryAllocateInfo memAllocInfo{
        .allocationSize  = bufferMemReq.size,
        .memoryTypeIndex = find_memory_type_index(m_deviceMemoryProperties,
                                                  info.memoryProperties,
                                                  bufferMemReq.memoryTypeBits),
    };
    m_bufferMemory   = m_device.allocateMemory(memAllocInfo);
    m_bufferLocation = info.memoryProperties;
    m_device.bindBufferMemory(m_buffer, m_bufferMemory, 0);
}

void Buffer::copy_data(vk::DeviceSize offset, vk::DeviceSize size, void* dataSrc)
{
    EC_ASSERT(!(m_bufferLocation & vk::MemoryPropertyFlagBits::eDeviceLocal));
    void* dataDst;
    std::ignore = m_device.mapMemory(m_bufferMemory, offset, size, {}, &dataDst);
    memcpy(dataDst, dataSrc, static_cast<size_t>(size));
    m_device.unmapMemory(m_bufferMemory);
}

void Buffer::free()
{
    m_device.freeMemory(m_bufferMemory);
    m_device.destroyBuffer(m_buffer);
}

}  // namespace ec::vulkan
