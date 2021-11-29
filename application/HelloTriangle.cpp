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
    , m_GraphicsQueue(m_Device, getQueueFamilyIndeces(m_PhysicalDevice).graphicsFamily.value(), 0)
    , m_PresentQueue(m_Device, getQueueFamilyIndeces(m_PhysicalDevice).presentFamily.value(), 0)
    , m_SwapChain(createSwapChain())
    , m_ImageViews(createImageViews())
    , m_RenderPass(createRenderPass())
    , m_Pipeline(createPipeline())
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

    SwapChainSupportDetails swapChainSupport = getSwapChainSupportDetails();

    m_SwapChainImageFormat         = getSwapChainFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = getPresentMode(swapChainSupport.presentModes);
    m_SwapChainExtents             = get2DExtents(swapChainSupport.capabilities, m_Window.GetFrameBufferSize());

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo(
        {},
        static_cast<vk::SurfaceKHR>(*m_Surface),
        imageCount,
        m_SwapChainImageFormat.format,
        m_SwapChainImageFormat.colorSpace,
        m_SwapChainExtents,
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

vk::raii::RenderPass HelloTriangle::createRenderPass()
{
    std::array<vk::AttachmentDescription, 1> attachmentDescriptions;
    attachmentDescriptions[0] = vk::AttachmentDescription(
        {},
        m_SwapChainImageFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorReference(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::SubpassDescription  subpass({}, vk::PipelineBindPoint::eGraphics, {}, colorReference);

    vk::RenderPassCreateInfo renderPassCreateInfo({}, attachmentDescriptions, subpass);
    return vk::raii::RenderPass(m_Device, renderPassCreateInfo);
}

vk::raii::Pipeline HelloTriangle::createPipeline()
{
    Shader fragShader("shaders/fragment.frag.bin");
    Shader vertexShader("shaders/vertex.vert.bin");

    vk::raii::ShaderModule vertexShaderModule = vertexShader.GetShaderModule(m_Device);
    vk::raii::ShaderModule fragShaderModule   = fragShader.GetShaderModule(m_Device);

    std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, *vertexShaderModule, "main"),
        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, *fragShaderModule, "main")};

    vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
        {}, // flags
        0,
        0);

    vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
        {},
        vk::PrimitiveTopology::eTriangleList,
        VK_FALSE);

    std::vector<vk::Rect2D> scissor;
    scissor.push_back(vk::Rect2D(vk::Offset2D(0, 0), m_SwapChainExtents));
    std::vector<vk::Viewport> viewport;
    viewport.push_back(vk::Viewport(0.0f, 0.0f, m_SwapChainExtents.width, m_SwapChainExtents.height, 0.0f, 1.0f));

    vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo({}, viewport, scissor);

    vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
        {},                          // flags
        false,                       // depthClampEnable
        false,                       // rasterizerDiscardEnable
        vk::PolygonMode::eFill,      // polygonMode
        vk::CullModeFlagBits::eBack, // cullMode
        vk::FrontFace::eClockwise,   // frontFace
        false,                       // depthBiasEnable
        0.0f,                        // depthBiasConstantFactor
        0.0f,                        // depthBiasClamp
        0.0f,                        // depthBiasSlopeFactor
        1.0f                         // lineWidth
    );

    vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
        {},                         // flags
        vk::SampleCountFlagBits::e1 // rasterizationSamples
                                    // other values can be default
    );

    vk::StencilOpState stencilOpState(
        vk::StencilOp::eKeep,
        vk::StencilOp::eKeep,
        vk::StencilOp::eKeep,
        vk::CompareOp::eAlways);
    vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
        {},                          // flags
        true,                        // depthTestEnable
        true,                        // depthWriteEnable
        vk::CompareOp::eLessOrEqual, // depthCompareOp
        false,                       // depthBoundTestEnable
        false,                       // stencilTestEnable
        stencilOpState,              // front
        stencilOpState               // back
    );

    vk::ColorComponentFlags colorComponentFlags(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);
    vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
        false,                  // blendEnable
        vk::BlendFactor::eZero, // srcColorBlendFactor
        vk::BlendFactor::eZero, // dstColorBlendFactor
        vk::BlendOp::eAdd,      // colorBlendOp
        vk::BlendFactor::eZero, // srcAlphaBlendFactor
        vk::BlendFactor::eZero, // dstAlphaBlendFactor
        vk::BlendOp::eAdd,      // alphaBlendOp
        colorComponentFlags     // colorWriteMask
    );

    vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
        {
    },                                 // flags
        false,                             // logicOpEnable
        vk::LogicOp::eCopy,                // logicOp
        pipelineColorBlendAttachmentState, // attachments
        {{0.0f, 0.0f, 0.0f, 0.0f}}         // blendConstants
    );

    std::array<vk::DynamicState, 2>    dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo({}, dynamicStates);

    vk::PipelineLayoutCreateInfo pipeLineCreateInfo;
    vk::raii::PipelineLayout     pipelineLayout(m_Device, pipeLineCreateInfo);

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
        {},                                    // flags
        pipelineShaderStageCreateInfos,        // stages
        &pipelineVertexInputStateCreateInfo,   // pVertexInputState
        &pipelineInputAssemblyStateCreateInfo, // pInputAssemblyState
        nullptr,                               // pTessellationState
        &pipelineViewportStateCreateInfo,      // pViewportState
        &pipelineRasterizationStateCreateInfo, // pRasterizationState
        &pipelineMultisampleStateCreateInfo,   // pMultisampleState
        &pipelineDepthStencilStateCreateInfo,  // pDepthStencilState
        &pipelineColorBlendStateCreateInfo,    // pColorBlendState
        &pipelineDynamicStateCreateInfo,       // pDynamicState
        *pipelineLayout,                       // layout
        *m_RenderPass                          // renderPass
    );

    vk::raii::Pipeline pipeline(m_Device, nullptr, graphicsPipelineCreateInfo);

    if (pipeline.getConstructorSuccessCode() != vk::Result::eSuccess)
    {
        throw std::runtime_error("Falid to create Graphics Pipeline.");
    }
    return pipeline;
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

vk::Extent2D HelloTriangle::get2DExtents(
    vk::SurfaceCapabilitiesKHR  capabilities,
    GLFWWindow::FrameBufferSize frameBufferSize)
{

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
