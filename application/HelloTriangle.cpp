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
    , m_Device(createDevice())
    , m_SwapChain(createSwapChain())
    , m_ImageViews(createImageViews())
    , m_PipelineLayout(createPipelineLayout())
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

    std::vector<vk::LayerProperties>     instanceLayerProperties     = context.enumerateInstanceLayerProperties();
    std::vector<vk::ExtensionProperties> instanceExtensionProperties = context.enumerateInstanceExtensionProperties();

    // initialize the vk::ApplicationInfo structure
    vk::ApplicationInfo applicationInfo(
        AppName.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        EngineName.c_str(),
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_0);

    std::vector<const char*> extensions = window.GetExtensions();
    for (const auto& extensionProperties : instanceExtensionProperties)
    {
        if (strcmp(extensionProperties.extensionName, "VK_KHR_get_physical_device_properties2"))
        {
            m_EnablePortabilityExtension = true;
            extensions.push_back("VK_KHR_get_physical_device_properties2");
        }
    }

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

vk::raii::Device HelloTriangle::createDevice()
{
    QueueFamilyIndices indexes = getQueueFamilyIndeces(m_PhysicalDevice);

    float                     queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, indexes.graphicsFamily.value(), 1, &queuePriority);

    std::vector<const char*> validationLayers;
    std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    if (m_EnablePortabilityExtension)
    {
        extensions.push_back("VK_KHR_portability_subset");
    }

    if constexpr (ENABLE_VALIDATION)
    {
        validationLayers = VulkanDebugLog::GetLayers();
    }

    if (!areDeviceExtensionsSupported(extensions, m_PhysicalDevice))
    {
        throw("Required device extensions are not supported.");
    }

    vk::PhysicalDeviceFeatures physicalFeatures;

    vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfo, validationLayers, extensions, &physicalFeatures);

    return vk::raii::Device(m_PhysicalDevice, deviceCreateInfo);
}

HelloTriangle::SwapChainSupportDetails HelloTriangle::getSwapChainSupportDetails()
{
    SwapChainSupportDetails swapChainDetails;

    swapChainDetails.formats      = m_PhysicalDevice.getSurfaceFormatsKHR(static_cast<vk::SurfaceKHR>(*m_Surface));
    swapChainDetails.presentModes = m_PhysicalDevice.getSurfacePresentModesKHR(static_cast<vk::SurfaceKHR>(*m_Surface));
    if (swapChainDetails.formats.empty() || swapChainDetails.presentModes.empty())
    {
        throw std::runtime_error("Device must have both valid formats and presentModes.");
    }

    swapChainDetails.capabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(static_cast<vk::SurfaceKHR>(*m_Surface));

    return swapChainDetails;
}

vk::raii::SwapchainKHR HelloTriangle::createSwapChain()
{
    auto getPresentMode = [](const std::vector<vk::PresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox)
            {
                return availablePresentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    };

    auto get2DExtents = [](vk::SurfaceCapabilitiesKHR capabilities, GLFWWindow::FrameBufferSize frameBufferSize) {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            vk::Extent2D actualExtent(
                static_cast<uint32_t>(frameBufferSize.Width),
                static_cast<uint32_t>(frameBufferSize.Height));

            actualExtent.width =
                std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height =
                std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    };

    SwapChainSupportDetails swapChainSupport = getSwapChainSupportDetails();

    vk::SurfaceFormatKHR surfaceFormat = getSwapChainFormat(swapChainSupport.formats);
    vk::PresentModeKHR   presentMode   = getPresentMode(swapChainSupport.presentModes);
    vk::Extent2D         extents       = get2DExtents(swapChainSupport.capabilities, m_Window.GetFrameBufferSize());

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo(
        {},
        static_cast<vk::SurfaceKHR>(*m_Surface),
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extents,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        vk::SharingMode::eExclusive,
        {},
        {},
        swapChainSupport.capabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        presentMode,
        VK_TRUE,
        VK_NULL_HANDLE);

    return vk::raii::SwapchainKHR(m_Device, createInfo);
}

std::vector<vk::raii::ImageView> HelloTriangle::createImageViews()
{
    std::vector<vk::raii::ImageView> imageViews;

    SwapChainSupportDetails swapChainSupport = getSwapChainSupportDetails();
    std::vector<VkImage>    swapChainImages  = m_SwapChain.getImages();
    vk::SurfaceFormatKHR    surfaceFormat    = getSwapChainFormat(swapChainSupport.formats);

    imageViews.reserve(swapChainImages.size());
    vk::ImageViewCreateInfo imageViewCreateInfo(
        {},
        {},
        vk::ImageViewType::e2D,
        surfaceFormat.format,
        {},
        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

    for (auto image : swapChainImages)
    {
        imageViewCreateInfo.image = static_cast<vk::Image>(image);
        imageViews.push_back({m_Device, imageViewCreateInfo});
    }

    return imageViews;
}

vk::raii::PipelineLayout HelloTriangle::createPipelineLayout()
{
    Shader fragShader("shaders/fragment.frag.bin");
    Shader vertexShader("shaders/vertex.vert.bin");

    vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding;

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    return vk::raii::PipelineLayout(m_Device, pipelineLayoutCreateInfo);
}

vk::SurfaceFormatKHR HelloTriangle::getSwapChainFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    throw std::runtime_error("could not find valid surface format.");
}

bool HelloTriangle::areDeviceExtensionsSupported(
    std::vector<const char*>        extensions,
    const vk::raii::PhysicalDevice& physicalDevice)
{
    auto deviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    for (auto& extension : deviceExtensions)
    {
        std::string extensionName(extension.extensionName);

        auto it = extensions.begin();
        while (it != extensions.end())
        {
            if (std::string(*it) == extensionName)
            {
                break; // While()
            }
            ++it;
        }
        if (it != extensions.end())
        {
            extensions.erase(it);
        }

        if (extensions.empty())
        {
            break; // For()
        }
    }

    return extensions.empty();
}
