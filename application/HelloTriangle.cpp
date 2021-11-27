#include "HelloTriangle.h"

#include <iostream>

const std::string HelloTriangle::AppName    = "HelloTriangle";
const std::string HelloTriangle::EngineName = "No Engine";

HelloTriangle::HelloTriangle()
    : m_Window(800, 600)
    , m_Instance(createInstance(m_Context, m_Window))
    , m_Surface(m_Window.CreateSurface(m_Instance))
#ifndef NDEBUG
    , m_Logger(
          m_Instance,
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
          vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
              vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
#endif
    , m_PhysicalDevice(getPhysicalDevice(m_Instance))
    , m_Device(createDevice(m_PhysicalDevice))
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

vk::raii::PhysicalDevice HelloTriangle::getPhysicalDevice(const vk::raii::Instance& instance)
{
    auto physicalDevices = vk::raii::PhysicalDevices(instance);
    for (auto& physicalDevice : physicalDevices)
    {
        QueueFamilyIndices indexes = getQueueFamilyIndeces(physicalDevice);
        if (indexes.isComplete())
        {
            return std::move(physicalDevice);
        }
    }

    throw std::runtime_error("Failed to find a valid phsycial device.");
}

HelloTriangle::QueueFamilyIndices HelloTriangle::getQueueFamilyIndeces(const vk::raii::PhysicalDevice& physicalDevice)
{
    QueueFamilyIndices indexes;
    int                i = 0;
    for (const auto& queueFamily : physicalDevice.getQueueFamilyProperties())
    {
        // We are going to ignore devices that don't support both present & graphics.
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics &&
            physicalDevice.getSurfaceSupportKHR(i, static_cast<vk::SurfaceKHR>(*m_Surface)))
        {
            indexes.graphicsFamily = i;
            indexes.presentFamily  = i;
        }

        if (indexes.isComplete())
        {
            return indexes;
        }
        ++i;
    }

    return indexes;
}

vk::raii::Device HelloTriangle::createDevice(const vk::raii::PhysicalDevice& physicalDevice)
{
    QueueFamilyIndices indexes = getQueueFamilyIndeces(physicalDevice);

    float                     queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, indexes.graphicsFamily.value(), 1, &queuePriority);

    auto deviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();

    std::vector<const char*> validationLayers;
    std::vector<const char*> extensions;
    if constexpr (ENABLE_VALIDATION)
    {
        // validationLayers = VulkanDebugLog::GetLayers();
    }

    vk::PhysicalDeviceFeatures physicalFeatures;

    vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfo, validationLayers, extensions, &physicalFeatures);

    return vk::raii::Device(physicalDevice, deviceCreateInfo);
}
