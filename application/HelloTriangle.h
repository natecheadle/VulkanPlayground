#pragma once

#include "GLFWWindow.h"
#include "VulkanDebugLog.h"

#include <vulkan/vulkan_raii.hpp>

#include <string>

class HelloTriangle {

#ifdef NDEBUG
    static constexpr bool ENABLE_VALIDATION = false;
#else
    static constexpr bool ENABLE_VALIDATION = true;
#endif

    static const std::string AppName;
    static const std::string EngineName;

    GLFWWindow m_Window;

    vk::raii::Context  m_Context;
    vk::raii::Instance m_Instance;

#ifndef NDEBUG
    VulkanDebugLog m_Logger;
#endif

  public:
    HelloTriangle();
    ~HelloTriangle();

    void Run();

    static bool checkLayers(std::vector<char const*> const& layers, std::vector<vk::LayerProperties> const& properties)
    {
        // return true if all layers are listed in the properties
        return std::all_of(layers.begin(), layers.end(), [&properties](char const* name) {
            return std::find_if(properties.begin(), properties.end(), [&name](vk::LayerProperties const& property) {
                       return strcmp(property.layerName, name) == 0;
                   }) != properties.end();
        });
    }

  private:
    static vk::raii::Instance createInstance(const vk::raii::Context& context, const GLFWWindow& window);
};