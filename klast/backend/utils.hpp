#pragma once

#include <ranges>
#include <vulkan/vulkan.hpp>

namespace klast::vulkan
{

bool contains_all_names(const std::vector<const char*>& bigSet,
                        const std::vector<const char*>& smallSet);

uint32_t find_memory_type_index(vk::PhysicalDeviceMemoryProperties availableMemoryProperties,
                                vk::MemoryPropertyFlags selectedMemoryProperties,
                                uint32_t allowedMemoryTypes);

}  // namespace klast::vulkan