#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

class GLFWWindow {
  public:
    static constexpr int DEFAULT_WIDTH  = 800;
    static constexpr int DEFAULT_HEIGHT = 600;

  private:
    GLFWwindow* m_pWindow;

    int m_Width;
    int m_Height;

    const std::vector<const char*> m_RequiredExtensions;

  public:
    GLFWWindow();
    GLFWWindow(int width, int height);
    ~GLFWWindow();

    const std::vector<const char*>& GetAppExtensions() const { return m_RequiredExtensions; }
    std::vector<const char*>&       AppendAppExtensions(std::vector<const char*>& extensions) const;

  private:
    static GLFWwindow*              createWindow(int width, int height);
    static std::vector<const char*> createExtensionList();
};