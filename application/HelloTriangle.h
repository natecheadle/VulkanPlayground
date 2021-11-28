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
            , presentFamily(0)
        {
        }
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR        capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR>   presentModes;
    };

#ifdef NDEBUG
    static constexpr bool ENABLE_VALIDATION = false;
#else
    static constexpr bool ENABLE_VALIDATION = true;
#endif

    static const std::string AppName;
    static const std::string EngineName;

    bool m_EnablePortabilityExtension = false;

    GLFWWindow m_Window;

    vk::raii::Context    m_Context;
    vk::raii::Instance   m_Instance;
    vk::raii::SurfaceKHR m_Surface;

#ifndef NDEBUG
    VulkanDebugLog m_Logger;
#endif

    vk::raii::PhysicalDevice m_PhysicalDevice;
    vk::raii::Device         m_Device;

    vk::raii::SwapchainKHR           m_SwapChain;
    std::vector<vk::raii::ImageView> m_ImageViews;

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
    vk::raii::Instance               createInstance(const vk::raii::Context& context, const GLFWWindow& window);
    QueueFamilyIndices               getQueueFamilyIndeces(const vk::raii::PhysicalDevice& physicalDevice);
    vk::raii::PhysicalDevice         getPhysicalDevice(const vk::raii::Instance& instance);
    vk::raii::Device                 createDevice();
    SwapChainSupportDetails          getSwapChainSupportDetails();
    vk::raii::SwapchainKHR           createSwapChain();
    std::vector<vk::raii::ImageView> createImageViews();

    static vk::SurfaceFormatKHR getSwapChainFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static bool                 areDeviceExtensionsSupported(
                        std::vector<const char*>        extensions,
                        const vk::raii::PhysicalDevice& physicalDevice);
};