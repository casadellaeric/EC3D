#pragma once

#pragma warning(push)
#pragma warning(disable : 4623 5045 6285 26450 26451 26495 26498 26800)

#define NOMINMAX

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
#include <iostream>
#include <utility>
#include <thread>

// 3rd party
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

// Klast
#include "misc/assertion.hpp"
#include "misc/logger.hpp"

#pragma warning(pop)
