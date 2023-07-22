#pragma once

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include <string_view>

namespace klast
{

// For now, only one non-resizable window that controls the glfw context
class Window
{
public:

    struct InitInfo {
        int width;
        int height;
        std::string_view name;
    };

public:

    ~Window();

    Window& operator=(Window&& other) noexcept;

    void init(const Window::InitInfo& initInfo);

    GLFWwindow* get_handle() const;
    const auto get_required_extensions() const;
    bool close_signalled() const;
    void poll_events() const;

private:

    GLFWwindow* create_window_glfw(const Window::InitInfo& initInfo);

private:

    GLFWwindow* m_handle{ nullptr };
    int m_width{};
    int m_height{};
};

}  // namespace klast
