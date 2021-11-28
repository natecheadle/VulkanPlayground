#pragma once

#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>

#include <functional>
#include <string>
#include <vector>

class GLFWWindow {
  public:
    static constexpr int DEFAULT_WIDTH  = 800;
    static constexpr int DEFAULT_HEIGHT = 600;

    struct FrameBufferSize
    {
        int Height;
        int Width;
    };

  private:
    GLFWwindow* m_pWindow;

    int m_Width;
    int m_Height;

    const std::vector<const char*> m_RequiredExtensions;
    std::function<void()>          f_RunWindow;

  public:
    GLFWWindow();
    GLFWWindow(int width, int height);
    ~GLFWWindow();

    const std::vector<const char*>& GetExtensions() const { return m_RequiredExtensions; }
    std::vector<const char*>&       AppendExtensions(std::vector<const char*>& extensions) const;

    vk::raii::SurfaceKHR CreateSurface(const vk::raii::Instance& instance);

    FrameBufferSize GetFrameBufferSize();

    virtual bool ShouldClose() { return glfwWindowShouldClose(m_pWindow); }
    void         SetMainLoop(const std::function<void()>& f_mainLoop) { f_RunWindow = f_mainLoop; }
    void         Run()
    {
        glfwPollEvents();
        f_RunWindow();
    }

  private:
    static GLFWwindow*              createWindow(int width, int height);
    static std::vector<const char*> createExtensionList();
};