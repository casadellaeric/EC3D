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

    struct Info {
        int width;
        int height;
        std::string_view name;
    };

    Window() = default;
    explicit Window(const Window::Info& info);
    ~Window() noexcept;

    Window(const Window&)            = delete;
    Window(Window&&)                 = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&)      = default;

    void free();

    GLFWwindow* get_handle() const { return m_handle; };
    int get_width() const { return m_width; };
    int get_height() const { return m_height; };
    vk::SurfaceKHR get_surface() const { return m_surface; };
    std::tuple<int, int> get_framebuffer_size() const;
    std::vector<const char*> get_required_extensions() const;
    bool close_signalled() const { return glfwWindowShouldClose(m_handle); };
    void poll_events() const { glfwPollEvents(); };

    void create_surface(vk::Instance instance);
    void delete_surface() { m_surface = nullptr; };

private:

    GLFWwindow* m_handle{ nullptr };
    int m_width{};
    int m_height{};

    vk::SurfaceKHR m_surface;
};

static GLFWwindow* create_window_glfw(int width, int height, std::string_view windowName);

}  // namespace klast
