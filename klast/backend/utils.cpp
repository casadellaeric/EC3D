#include "pch.hpp"

#include "utils.hpp"
#include <fstream>
#include <ranges>

namespace klast::vulkan
{

bool contains_all_names(const std::vector<const char*>& bigSet,
                        const std::vector<const char*>& smallSet)
{
    // Convert bigSet for easy comparison with std::ranges::find
    std::vector<std::string_view> bigSetSv;
    std::ranges::transform(bigSet,
                           std::back_inserter(bigSetSv),
                           [](const char* c) { return std::string_view(c); });

    return std::ranges::all_of(
        smallSet,
        [&](const char* c)
        { return std::ranges::find(bigSetSv, std::string_view(c)) != bigSetSv.end(); });
}

uint32_t find_memory_type_index(vk::PhysicalDeviceMemoryProperties availableMemoryProperties,
                                vk::MemoryPropertyFlags selectedMemoryProperties,
                                uint32_t allowedMemoryTypes)
{
    for (uint32_t i = 0; i < availableMemoryProperties.memoryTypeCount; ++i) {
        if ((allowedMemoryTypes & (1 << i))
            && ((availableMemoryProperties.memoryTypes[i].propertyFlags & selectedMemoryProperties)
                == selectedMemoryProperties)) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find memory type with requested properties!");
}

std::vector<uint32_t> read_file(std::string_view filePath)
{
    std::ifstream file(filePath.data(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Failed to open file {}\n", filePath));
    }

    size_t fileSize{ static_cast<size_t>(file.tellg()) };
    std::vector<uint32_t> fileData(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read((char*)fileData.data(), fileSize);
    file.close();

    return fileData;
}

}  // namespace klast::vulkan