#include "VulkanDebugLog.h"

#include <iostream>
#include <stdexcept>

PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugUtilsMessengerEXT*                 pMessenger)
{
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance                   instance,
    VkDebugUtilsMessengerEXT     messenger,
    VkAllocationCallbacks const* pAllocator)
{
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

VulkanDebugLog::VulkanDebugLog(
    const vk::raii::Instance&             instance,
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags,
    vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags)
    : m_CreateInfo({}, severityFlags, messageTypeFlags, &DebugMessageFunc)
    , m_DebugMessenger(CreateDebugMessenger(instance, m_CreateInfo))
{
}

VulkanDebugLog::~VulkanDebugLog() {}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugLog::DebugMessageFunc(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
    void* /*pUserData*/)
{
    std::string message;

    message += vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) + ": " +
               vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) + ":\n";
    message += std::string("\t") + "messageIDName   = <" + pCallbackData->pMessageIdName + ">\n";
    message += std::string("\t") + "messageIdNumber = " + std::to_string(pCallbackData->messageIdNumber) + "\n";
    message += std::string("\t") + "message         = <" + pCallbackData->pMessage + ">\n";
    if (0 < pCallbackData->queueLabelCount)
    {
        message += std::string("\t") + "Queue Labels:\n";
        for (uint8_t i = 0; i < pCallbackData->queueLabelCount; i++)
        {
            message += std::string("\t\t") + "labelName = <" + pCallbackData->pQueueLabels[i].pLabelName + ">\n";
        }
    }
    if (0 < pCallbackData->cmdBufLabelCount)
    {
        message += std::string("\t") + "CommandBuffer Labels:\n";
        for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
        {
            message += std::string("\t\t") + "labelName = <" + pCallbackData->pCmdBufLabels[i].pLabelName + ">\n";
        }
    }
    if (0 < pCallbackData->objectCount)
    {
        for (uint8_t i = 0; i < pCallbackData->objectCount; i++)
        {
            message += std::string("\t") + "Object " + std::to_string(i) + "\n";
            message += std::string("\t\t") + "objectType   = " +
                       vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) + "\n";
            message += std::string("\t\t") +
                       "objectHandle = " + std::to_string(pCallbackData->pObjects[i].objectHandle) + "\n";
            if (pCallbackData->pObjects[i].pObjectName)
            {
                message += std::string("\t\t") + "objectName   = <" + pCallbackData->pObjects[i].pObjectName + ">\n";
            }
        }
    }

    std::cout << message << std::endl;

    return false;
}
vk::raii::DebugUtilsMessengerEXT VulkanDebugLog::CreateDebugMessenger(
    const vk::raii::Instance&                   instance,
    const vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
    pfnVkCreateDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    if (!pfnVkCreateDebugUtilsMessengerEXT)
    {
        throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");
    }

    pfnVkDestroyDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
    if (!pfnVkDestroyDebugUtilsMessengerEXT)
    {
        throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");
    }

    return vk::raii::DebugUtilsMessengerEXT(instance, createInfo);
}
