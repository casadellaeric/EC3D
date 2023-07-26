#pragma once

#include <ranges>
#include <vulkan/vulkan.hpp>

namespace klast::vulkan
{

bool contains_all_names(const std::vector<const char*>& bigSet,
                        const std::vector<const char*>& smallSet);

}  // namespace klast::vulkan