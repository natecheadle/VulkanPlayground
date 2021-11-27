#pragma once

#include <vulkan/vulkan_raii.hpp>

class VulkanDebugLog {
    vk::DebugUtilsMessengerCreateInfoEXT m_CreateInfo;
    vk::raii::DebugUtilsMessengerEXT     m_DebugMessenger;

  public:
    VulkanDebugLog(
        const vk::raii::Instance&             instance,
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags,
        vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags);
    ~VulkanDebugLog();

    static std::vector<const char*> GetAppExtensions()
    {
        std::vector<const char*> extensions;
        extensions.push_back("VK_LAYER_KHRONOS_validation");
        return extensions;
    }

    static std::vector<const char*>& AppendAppExtensions(std::vector<const char*>& extensions)
    {
        extensions.push_back("VK_LAYER_KHRONOS_validation");
        return extensions;
    }

    static std::vector<const char*> GetEnabledLayers()
    {
        std::vector<const char*> layers;
        layers.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        return layers;
    }

    static std::vector<const char*>& AppendEnabledLayers(std::vector<const char*>& layers)
    {
        layers.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        return layers;
    }

  private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessageFunc(
        VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
        VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
        void* /*pUserData*/);

    static vk::raii::DebugUtilsMessengerEXT CreateDebugMessenger(
        const vk::raii::Instance&                   instance,
        const vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
};
