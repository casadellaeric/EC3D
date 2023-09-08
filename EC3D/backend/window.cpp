#include "pch.hpp"
#include "window.hpp"

namespace ec
{

Window::Window(const Window::Info& info) :
  m_handle(create_window_glfw(info.width, info.height, info.name)),
  m_width(info.width),
  m_height(info.height)
{
}

Window::~Window() { }

void Window::free()
{
    if (!m_handle) {
        return;
    }
    glfwDestroyWindow(m_handle);
    glfwTerminate();  // TODO: Don't terminate if in a multi-window application
}

std::tuple<int, int> Window::get_framebuffer_size() const
{
    int width, height;
    glfwGetFramebufferSize(m_handle, &width, &height);
    return std::make_tuple(width, height);
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

void Window::create_surface(vk::Instance instance)
{
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance, m_handle, nullptr, &surface);
    m_surface = vk::SurfaceKHR(surface);
}

GLFWwindow* create_window_glfw(int width, int height, std::string_view windowName)
{
    EC_ASSERT(width > 0 && height > 0);
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

}  // namespace ec
