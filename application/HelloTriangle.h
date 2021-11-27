#pragma once

#include "GLFWWindow.h"
#include "VulkanDebugLog.h"

#include <vulkan/vulkan_raii.hpp>

#include <optional>
#include <string>

class HelloTriangle {
    struct QueueFamilyIndices
    {
        QueueFamilyIndices()
            : graphicsFamily(0)
        {
        }
        std::optional<uint32_t> graphicsFamily;

        bool isComplete() { return graphicsFamily.has_value(); }
    };

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
    vk::raii::PhysicalDevice m_PhysicalDevice;
    vk::raii::Device         m_Device;

  public:
    HelloTriangle();
    ~HelloTriangle();

    void Run();

    static bool checkLayers(std::vector<const char*> const& layers, std::vector<vk::LayerProperties> const& properties)
    {
        // return true if all layers are listed in the properties
        return std::all_of(layers.begin(), layers.end(), [&properties](const char* name) {
            return std::find_if(properties.begin(), properties.end(), [&name](vk::LayerProperties const& property) {
                       return strcmp(property.layerName, name) == 0;
                   }) != properties.end();
        });
    }

  private:
    static vk::raii::Instance       createInstance(const vk::raii::Context& context, const GLFWWindow& window);
    static vk::raii::PhysicalDevice getPhysicalDevice(const vk::raii::Instance& instance);
    static QueueFamilyIndices       getQueueFamilyIndeces(const vk::raii::PhysicalDevice& physicalDevice);
    static vk::raii::Device         createDevice(const vk::raii::PhysicalDevice& physicalDevice);
};