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

    static std::vector<const char*> GetExtensions()
    {
        std::vector<const char*> extensions;
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        return extensions;
    }

    static std::vector<const char*>& AppendExtensions(std::vector<const char*>& extensions)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        return extensions;
    }

    static std::vector<const char*> GetLayers()
    {
        std::vector<const char*> layers;
        layers.push_back("VK_LAYER_KHRONOS_validation");
        return layers;
    }

    static std::vector<const char*>& AppendLayers(std::vector<const char*>& layers)
    {
        layers.push_back("VK_LAYER_KHRONOS_validation");
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
