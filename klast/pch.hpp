#pragma once

#pragma warning(push)
#pragma warning(disable : 4623 5045 6285 26450 26451 26495 26498 26800)

#ifdef _WIN32
    #include <Windows.h>
#endif

// STL
#include <array>
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

// 3rd party
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

// Klast
#include "utils/assertion.hpp"
#include "utils/logger.hpp"

#pragma warning(pop)