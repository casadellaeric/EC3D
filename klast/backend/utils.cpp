#include <pch.hpp>

#include "utils.hpp"
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

}  // namespace klast::vulkan