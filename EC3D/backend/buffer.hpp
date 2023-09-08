#pragma once

namespace ec::vulkan
{

class Buffer
{
    friend class Device;
    friend class GraphicsContext;

public:

    struct Info {
        uint32_t count;
        vk::DeviceSize elemSize;
        vk::BufferUsageFlags usage;
        vk::MemoryPropertyFlags memoryProperties;
        vk::IndexType indexType{ vk::IndexType::eNoneKHR };
    };

    Buffer(const Buffer&) = default;
    Buffer(Buffer&&)      = default;

    vk::IndexType get_index_type() const { return m_indexType; }
    uint32_t get_count() const { return m_bufferCount; }
    vk::DeviceSize get_size() const { return m_bufferByteSize; }

    void copy_data(vk::DeviceSize offset, vk::DeviceSize size, void* data);

private:

    Buffer(vk::Device device,
           const vk::PhysicalDeviceMemoryProperties& deviceMemoryProperties,
           const std::vector<uint32_t>& queueFamilies,
           const Buffer::Info& info);

    void create(const Buffer::Info& info, const std::vector<uint32_t>& queueFamilies);

    void free();

    inline vk::Buffer get_handle() const { return m_buffer; }

private:

    vk::Buffer m_buffer;
    vk::DeviceMemory m_bufferMemory;
    vk::MemoryPropertyFlags m_bufferLocation;

    uint32_t m_bufferCount{};
    vk::DeviceSize m_bufferByteSize{};

    // Only useful in index buffers
    vk::IndexType m_indexType{ vk::IndexType::eNoneKHR };

    vk::Device m_device;
    const vk::PhysicalDeviceMemoryProperties& m_deviceMemoryProperties;
};

}  // namespace ec::vulkan
