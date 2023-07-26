#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include <string_view>
#include <vector>

namespace klast
{

// For now, only one non-resizable window that controls the glfw context
class Window
{
public:

    struct CreateInfo {
        int width;
        int height;
        std::string_view name;
    };

    explicit Window(const Window::CreateInfo& createInfo);
    ~Window() noexcept;

    Window(const Window&)            = delete;
    Window(Window&&)                 = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&)      = delete;

public:

    void free();

    GLFWwindow* get_handle() const { return m_handle; };
    std::vector<const char*> get_required_extensions() const;
    bool close_signalled() const { return glfwWindowShouldClose(m_handle); };
    void poll_events() const { glfwPollEvents(); };
    void create_surface();

private:

    GLFWwindow* m_handle{ nullptr };
    int m_width{};
    int m_height{};

    vk::SurfaceKHR m_surface;
};

static GLFWwindow* create_window_glfw(int width, int height, std::string_view windowName);

}  // namespace klast
