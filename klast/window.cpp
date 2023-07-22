#include "pch.hpp"
#include "window.hpp"

namespace klast
{

Window::~Window()
{
    if (m_handle != nullptr) {
        glfwDestroyWindow(m_handle);
        glfwTerminate();
    }
}

Window& Window::operator=(Window&& other) noexcept
{
    m_width        = other.m_width;
    m_height       = other.m_height;
    m_handle       = other.m_handle;
    other.m_handle = nullptr;  // Other window no longer manages this window when moving
    return *this;
}

void Window::init(const Window::InitInfo& initInfo)
{
    m_handle = create_window_glfw(initInfo);
    m_width  = initInfo.width;
    m_height = initInfo.height;
}

GLFWwindow* Window::get_handle() const
{
    return m_handle;
}

bool Window::close_signalled() const
{
    return glfwWindowShouldClose(m_handle) == GLFW_TRUE;
}

void Window::poll_events() const
{
    glfwPollEvents();
}

const auto Window::get_required_extensions() const
{
    uint32_t extensionCount{};
    const char** requiredExtensions{ glfwGetRequiredInstanceExtensions(&extensionCount) };
    if (requiredExtensions == nullptr) {
        throw std::runtime_error("Failed to obtain Vulkan instance extensions. GLFW is unable to "
                                 "create window surfaces!");
    }

    return std::vector<const char*>(requiredExtensions, requiredExtensions + extensionCount);
}

GLFWwindow* Window::create_window_glfw(const Window::InitInfo& initInfo)
{
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Not needed if not using OpenGL or OpenGL ES
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // TODO: Allow resizable window

    GLFWwindow* windowHandle
        = glfwCreateWindow(initInfo.width, initInfo.height, initInfo.name.data(), nullptr, nullptr);
    if (windowHandle == nullptr) {
        throw std::runtime_error("Failed to create a window!");
    }

    return windowHandle;
}

}  // namespace klast
