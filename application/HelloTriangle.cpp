#include "HelloTriangle.h"

const std::string HelloTriangle::AppName    = "HelloTriangle";
const std::string HelloTriangle::EngineName = "No Engine";

HelloTriangle::HelloTriangle()
    : m_Window(800, 600)
    , m_Instance(createInstance(m_Context, m_Window))
#ifndef NDEBUG
    , m_Logger(
          m_Instance,
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
          vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
              vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
#endif
{
}

HelloTriangle::~HelloTriangle() {}

void HelloTriangle::Run() {}

vk::raii::Instance HelloTriangle::createInstance(const vk::raii::Context& context, const GLFWWindow& window)
{

    std::vector<vk::LayerProperties> instanceLayerProperties = context.enumerateInstanceLayerProperties();

    // initialize the vk::ApplicationInfo structure
    vk::ApplicationInfo applicationInfo(
        AppName.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        EngineName.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0);

    std::vector<const char*> extensions = window.GetAppExtensions();

    vk::InstanceCreateInfo instanceCreateInfo;

    // initialize the vk::InstanceCreateInfo
    if constexpr (ENABLE_VALIDATION)
    {
        std::vector<const char*> validationLayers;
        VulkanDebugLog::AppendAppExtensions(extensions);

        if (!checkLayers(extensions, instanceLayerProperties))
        {
            throw std::runtime_error(
                "Set the environment variable VK_LAYER_PATH to point to the location of your layers");
        }

        VulkanDebugLog::AppendEnabledLayers(validationLayers);

        instanceCreateInfo = vk::InstanceCreateInfo(
            {},
            &applicationInfo,
            static_cast<uint32_t>(extensions.size()),
            extensions.data(),
            static_cast<uint32_t>(validationLayers.size()),
            validationLayers.data());
    }
    else
    {
        if (!checkLayers(extensions, instanceLayerProperties))
        {
            throw std::runtime_error(
                "Set the environment variable VK_LAYER_PATH to point to the location of your layers");
        }

        vk::InstanceCreateInfo instanceCreateInfo = vk::InstanceCreateInfo(
            {},
            &applicationInfo,
            static_cast<uint32_t>(extensions.size()),
            extensions.data(),
            0,
            {});
    }

    return vk::raii::Instance(context, instanceCreateInfo);
}
