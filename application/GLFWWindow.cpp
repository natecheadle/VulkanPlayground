#include "GLFWWindow.h"

GLFWWindow::GLFWWindow()
    : m_pWindow(createWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT))
    , m_Width(DEFAULT_WIDTH)
    , m_Height(DEFAULT_HEIGHT)
    , m_RequiredExtensions(createExtensionList())
{
}

GLFWWindow::GLFWWindow(int width, int height)
    : m_pWindow(createWindow(width, height))
    , m_Width(width)
    , m_Height(height)
    , m_RequiredExtensions(createExtensionList())
{
}

GLFWWindow::~GLFWWindow()
{
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

std::vector<const char*>& GLFWWindow::AppendExtensions(std::vector<const char*>& extensions) const
{
    for (const auto& extension : m_RequiredExtensions)
    {
        extensions.push_back(extension);
    }

    return extensions;
}

GLFWwindow* GLFWWindow::createWindow(int width, int height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
}

std::vector<const char*> GLFWWindow::createExtensionList()
{
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}