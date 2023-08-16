#pragma once

namespace klast::vulkan
{

class Buffer
{
public:

    struct Info {
        vk::DeviceSize size;
        vk::BufferUsageFlags usage;
        vk::MemoryPropertyFlags memoryProperties;
        const vk::PhysicalDeviceMemoryProperties& deviceMemoryProperties;
    };

    Buffer(vk::Device device, const Buffer::Info& info);

    void free();

private:

    void create(const Buffer::Info& info);

private:

    vk::Buffer m_buffer;
    vk::DeviceMemory m_bufferMemory;

    const vk::Device* m_device;
};

}  // namespace klast::vulkan
