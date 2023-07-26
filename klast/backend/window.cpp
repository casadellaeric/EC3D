#include "pch.hpp"
#include "window.hpp"

namespace klast
{
Window::Window(const Window::CreateInfo& createInfo) :
  m_handle(create_window_glfw(createInfo.width, createInfo.height, createInfo.name)),
  m_width(createInfo.width),
  m_height(createInfo.height)
{
}

Window::~Window()
{
    free();
}

void Window::free()
{
    if (!m_handle) {
        return;
    }
    glfwDestroyWindow(m_handle);
    glfwTerminate();  // TODO: Don't terminate if in a multi-window application
}

std::vector<const char*> Window::get_required_extensions() const
{
    uint32_t extensionCount{};
    const char** requiredExtensions{ glfwGetRequiredInstanceExtensions(&extensionCount) };
    if (!requiredExtensions) {
        throw std::runtime_error("Failed to obtain Vulkan instance extensions. GLFW is unable to "
                                 "create window surfaces!");
    }

    return std::vector<const char*>(requiredExtensions, requiredExtensions + extensionCount);
}

GLFWwindow* create_window_glfw(int width, int height, std::string_view windowName)
{
    KL_ASSERT(width > 0 && height > 0);
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // Not needed if not using OpenGL or OpenGL ES
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // TODO: Allow resizable window

    GLFWwindow* windowHandle = glfwCreateWindow(width, height, windowName.data(), nullptr, nullptr);
    if (!windowHandle) {
        glfwTerminate();
        throw std::runtime_error("Failed to create a window!");
    }

    return windowHandle;
}

}  // namespace klast
