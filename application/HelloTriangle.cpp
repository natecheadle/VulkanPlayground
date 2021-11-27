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

void HelloTriangle::Run()
{

    while (!m_Window.ShouldClose())
    {
        m_Window.Run();
    }
}

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

    std::vector<const char*> extensions = window.GetExtensions();

    vk::InstanceCreateInfo instanceCreateInfo;

    std::vector<const char*> validationLayers;

    // initialize the vk::InstanceCreateInfo
    if constexpr (ENABLE_VALIDATION)
    {
        validationLayers = VulkanDebugLog::GetLayers();
        VulkanDebugLog::AppendExtensions(extensions);

        if (!checkLayers(validationLayers, instanceLayerProperties))
        {
            throw std::runtime_error(
                "Set the environment variable VK_LAYER_PATH to point to the location of your layers");
        }
    }

    instanceCreateInfo = vk::InstanceCreateInfo({}, &applicationInfo, validationLayers, extensions);

    return vk::raii::Instance(context, instanceCreateInfo);
}
