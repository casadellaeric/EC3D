#pragma once

#include <ranges>
#include <vulkan/vulkan.hpp>
#include <filesystem>

namespace ec::vulkan
{

bool contains_all_names(const std::vector<const char*>& bigSet,
                        const std::vector<const char*>& smallSet);

uint32_t find_memory_type_index(vk::PhysicalDeviceMemoryProperties availableMemoryProperties,
                                vk::MemoryPropertyFlags selectedMemoryProperties,
                                uint32_t allowedMemoryTypes);

std::vector<uint32_t> read_file(std::string_view filePath);

}  // namespace ec::vulkan