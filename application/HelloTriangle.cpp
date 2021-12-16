#include "HelloTriangle.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <chrono>
#include <iostream>
#include <set>

const std::string HelloTriangle::AppName    = "HelloTriangle";
const std::string HelloTriangle::EngineName = "No Engine";

std::array<vk::VertexInputBindingDescription, 1> HelloTriangle::Vertex::getBindingDescription()
{
    std::array<vk::VertexInputBindingDescription, 1> bindingDescriptions(
        {vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex)});

    return bindingDescriptions;
}

std::array<vk::VertexInputAttributeDescription, 3> HelloTriangle::Vertex::getAttributeDescriptions()
{
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {
        vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)),
        vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
        vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord))};

    return attributeDescriptions;
}

HelloTriangle::HelloTriangle()
    : m_Vertices({
          {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
          {{0.5f, -0.5f},  {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
          {{0.5f, 0.5f},   {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
          {{-0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
})
    , m_Indeces({0, 1, 2, 2, 3, 0})
    , m_Window(800, 600)
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
    , m_SwapChain(createSwapChain(VK_NULL_HANDLE))
    , m_ImageViews(createImageViews())
    , m_RenderPass(createRenderPass())
    , m_DescriptorLayout(createDescriptorSetLayout())
    , m_PipelineLayout(createPipelineLayout())
    , m_Pipeline(createPipeline())
    , m_Framebuffers(createFrameBuffers())
    , m_CommandPool(createCommandPool())
    , m_TextureImage(createTexture())
    , m_TextureImageView(createImageView(*(m_TextureImage.ImageData), vk::Format::eR8G8B8A8Srgb))
    , m_TextureSampler(createTextureSampler())
    , m_VertexBuffer(createVertexBuffer())
    , m_IndexBuffer(createIndexBuffer())
    , m_UniformBuffers(createUniformBuffers())
    , m_DescriptorPool(createDescriptorPool())
    , m_DescriptorSets(createDescriptorSets())
    , m_CommandBuffers(createCommandBuffers())
    , m_CurrentFrame(0)
    , m_FramebufferResized(false)
    , m_NewWidth(0)
    , m_NewHeight(0)
{
    vk::SemaphoreCreateInfo semaphoreInfo;
    vk::FenceCreateInfo     fenceInfo(vk::FenceCreateFlagBits::eSignaled);

    m_ImagesInFlight.resize(m_ImageViews.size(), nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_ImageAvailableSemaphores.push_back(vk::raii::Semaphore(m_Device, semaphoreInfo));
        m_RenderFinishedSemaphores.push_back(vk::raii::Semaphore(m_Device, semaphoreInfo));
        m_InFlightFences.push_back(vk::raii::Fence(m_Device, fenceInfo));
    }

    m_Window.SetOnResize([this](int width, int height) { this->onWindowResize(width, height); });
}

HelloTriangle::~HelloTriangle() {}

void HelloTriangle::Run()
{
    m_Window.SetMainLoop(std::bind(&HelloTriangle::drawFrame, this));
    while (!m_Window.ShouldClose())
    {
        m_Window.Run();
    }

    waitImagesInFlight();
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

bool checkDeviceExtensionSupport(vk::raii::PhysicalDevice& device)
{
    auto availableExtensions = device.enumerateDeviceExtensionProperties();

    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

vk::raii::PhysicalDevice HelloTriangle::getPhysicalDevice(const vk::raii::Instance& instance)
{
    auto physicalDevices = vk::raii::PhysicalDevices(instance);
    for (auto& physicalDevice : physicalDevices)
    {
        auto features = physicalDevice.getFeatures();

        bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = getSwapChainSupportDetails(physicalDevice);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        QueueFamilyIndices indexes = getQueueFamilyIndeces(physicalDevice);
        if (indexes.isComplete() && features.samplerAnisotropy)
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
    physicalFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfo, validationLayers, extensions, &physicalFeatures);

    return vk::raii::Device(m_PhysicalDevice, deviceCreateInfo);
}

HelloTriangle::SwapChainSupportDetails HelloTriangle::getSwapChainSupportDetails(
    vk::raii::PhysicalDevice& physicalDevice)
{
    SwapChainSupportDetails swapChainDetails;

    swapChainDetails.formats      = physicalDevice.getSurfaceFormatsKHR(static_cast<vk::SurfaceKHR>(*m_Surface));
    swapChainDetails.presentModes = physicalDevice.getSurfacePresentModesKHR(static_cast<vk::SurfaceKHR>(*m_Surface));
    if (swapChainDetails.formats.empty() || swapChainDetails.presentModes.empty())
    {
        throw std::runtime_error("Device must have both valid formats and presentModes.");
    }

    swapChainDetails.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(static_cast<vk::SurfaceKHR>(*m_Surface));

    return swapChainDetails;
}

vk::raii::SwapchainKHR HelloTriangle::createSwapChain(vk::SwapchainKHR oldSwapchain)
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

    SwapChainSupportDetails swapChainSupport = getSwapChainSupportDetails(m_PhysicalDevice);

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
        oldSwapchain);

    return vk::raii::SwapchainKHR(m_Device, createInfo);
}

std::vector<vk::raii::ImageView> HelloTriangle::createImageViews()
{
    std::vector<vk::raii::ImageView> imageViews;

    auto images = m_SwapChain.getImages();
    m_SwapChainImages.clear();
    for (auto& image : images)
    {
        m_SwapChainImages.push_back(std::move(static_cast<vk::Image>(image)));
    }

    SwapChainSupportDetails swapChainSupport = getSwapChainSupportDetails(m_PhysicalDevice);
    vk::SurfaceFormatKHR    surfaceFormat    = getSwapChainFormat(swapChainSupport.formats);

    imageViews.reserve(m_SwapChainImages.size());

    for (auto& image : m_SwapChainImages)
    {
        imageViews.push_back(createImageView(image, surfaceFormat.format));
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

vk::raii::PipelineLayout HelloTriangle::createPipelineLayout()
{
    std::array<vk::DescriptorSetLayout, 1> descriptorSets({*m_DescriptorLayout});

    vk::PipelineLayoutCreateInfo pipeLineCreateInfo({}, descriptorSets);
    return vk::raii::PipelineLayout(m_Device, pipeLineCreateInfo);
}

vk::raii::DescriptorSetLayout HelloTriangle::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding
        uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr);

    vk::DescriptorSetLayoutBinding samplerLayoutBinding(
        1,
        vk::DescriptorType::eCombinedImageSampler,
        1,
        vk::ShaderStageFlagBits::eFragment,
        nullptr);

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {
        std::move(uboLayoutBinding),
        std::move(samplerLayoutBinding)};
    vk::DescriptorSetLayoutCreateInfo createInfo({}, bindings);
    return vk::raii::DescriptorSetLayout(m_Device, createInfo, nullptr);
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

    auto bindingDescriptions   = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
        {}, // flags
        bindingDescriptions,
        attributeDescriptions);

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
        {},                               // flags
        false,                            // depthClampEnable
        false,                            // rasterizerDiscardEnable
        vk::PolygonMode::eFill,           // polygonMode
        vk::CullModeFlagBits::eBack,      // cullMode
        vk::FrontFace::eCounterClockwise, // frontFace
        false,                            // depthBiasEnable
        0.0f,                             // depthBiasConstantFactor
        0.0f,                             // depthBiasClamp
        0.0f,                             // depthBiasSlopeFactor
        1.0f                              // lineWidth
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
        *m_PipelineLayout,                     // layout
        *m_RenderPass                          // renderPass
    );

    vk::raii::Pipeline pipeline(m_Device, nullptr, graphicsPipelineCreateInfo);

    if (pipeline.getConstructorSuccessCode() != vk::Result::eSuccess)
    {
        throw std::runtime_error("Falid to create Graphics Pipeline.");
    }
    return pipeline;
}

std::vector<vk::raii::Framebuffer> HelloTriangle::createFrameBuffers()
{

    std::vector<vk::raii::Framebuffer> framebuffers;
    framebuffers.reserve(m_ImageViews.size());

    std::array<vk::ImageView, 1> attachments;

    for (auto const& view : m_ImageViews)
    {
        attachments[0] = *view;
        vk::FramebufferCreateInfo framebufferCreateInfo(
            {},
            *m_RenderPass,
            attachments,
            m_SwapChainExtents.width,
            m_SwapChainExtents.height,
            1);
        framebuffers.push_back(vk::raii::Framebuffer(m_Device, framebufferCreateInfo));
    }

    return framebuffers;
}

vk::raii::CommandPool HelloTriangle::createCommandPool()
{
    QueueFamilyIndices indexes = getQueueFamilyIndeces(m_PhysicalDevice);

    // create a CommandPool to allocate a CommandBuffer from
    vk::CommandPoolCreateInfo commandPoolCreateInfo({}, indexes.graphicsFamily.value());
    return vk::raii::CommandPool(m_Device, commandPoolCreateInfo);
}

vk::raii::CommandBuffers HelloTriangle::createCommandBuffers()
{
    // allocate a CommandBuffer from the CommandPool
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
        *m_CommandPool,
        vk::CommandBufferLevel::ePrimary,
        m_Framebuffers.size());
    vk::raii::CommandBuffers commandBuffers(m_Device, commandBufferAllocateInfo);

    std::array<vk::ClearValue, 1> clearValues;
    clearValues[0].color = vk::ClearColorValue(std::array<float, 4>({
        {0.0f, 0.0f, 0.0f, 1.0f}
    }));

    int i = 0;
    for (auto& commandBuffer : commandBuffers)
    {
        vk::RenderPassBeginInfo renderPassBeginInfo(
            *m_RenderPass,
            *m_Framebuffers[i],
            vk::Rect2D(vk::Offset2D(0, 0), m_SwapChainExtents),
            clearValues);

        std::array<vk::Buffer, 1>        vetexBuffers({*m_VertexBuffer.BufferData});
        std::array<vk::DeviceSize, 1>    deviceSizes({0});
        std::array<vk::DescriptorSet, 1> descriptorSets({*m_DescriptorSets[i]});
        commandBuffer.begin({});
        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_Pipeline);
        commandBuffer.setViewport(
            0,
            vk::Viewport(
                0.0f,
                0.0f,
                static_cast<float>(m_SwapChainExtents.width),
                static_cast<float>(m_SwapChainExtents.height),
                0.0f,
                1.0f));
        commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), m_SwapChainExtents));
        commandBuffer.bindVertexBuffers(0, vetexBuffers, deviceSizes);
        commandBuffer.bindIndexBuffer(*m_IndexBuffer.BufferData, 0, vk::IndexType::eUint16);
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            *m_PipelineLayout,
            0,
            descriptorSets,
            std::array<uint32_t, 0>());
        commandBuffer.drawIndexed(static_cast<uint32_t>(m_Indeces.size()), 1, 0, 0, 0);
        commandBuffer.endRenderPass();
        commandBuffer.end();

        ++i;
    }

    return std::move(commandBuffers);
}

vk::raii::Buffer HelloTriangle::createBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferFlags)
{
    vk::BufferCreateInfo bufferCreateInfo({}, bufferSize, bufferFlags, vk::SharingMode::eExclusive);
    return vk::raii::Buffer(m_Device, bufferCreateInfo);
}

HelloTriangle::ImageStruct HelloTriangle::createImage(
    uint32_t                width,
    uint32_t                height,
    vk::Format              format,
    vk::ImageTiling         tiling,
    vk::ImageUsageFlags     usage,
    vk::MemoryPropertyFlags properties)
{
    vk::ImageCreateInfo imageInfo(
        {},
        vk::ImageType::e2D,
        format,
        vk::Extent3D(width, height, 1),
        1,
        1,
        vk::SampleCountFlagBits::e1,
        tiling,
        usage,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::eUndefined);
    vk::raii::Image image(m_Device, imageInfo);

    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo(memRequirements.size, findMemoryType(memRequirements.memoryTypeBits, properties));

    vk::raii::DeviceMemory imageMemory(m_Device, allocInfo);
    image.bindMemory(*imageMemory, 0);

    return ImageStruct(std::move(image), std::move(imageMemory));
}

HelloTriangle::ImageStruct HelloTriangle::createTexture()
{
    int            texWidth, texHeight, texChannels;
    stbi_uc*       pixels    = stbi_load("textures/statue.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    vk::raii::Buffer       stagingBuffer       = createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc);
    vk::raii::DeviceMemory stagingBufferMemory = createDeviceMemory(
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer);

    void* data;
    data = stagingBufferMemory.mapMemory(0, imageSize, {});
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    stagingBufferMemory.unmapMemory();

    stbi_image_free(pixels);

    ImageStruct image = createImage(
        texWidth,
        texHeight,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    transitionImageLayout(
        image.ImageData,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal);

    copyBufferToImage(
        stagingBuffer,
        image.ImageData,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight));

    transitionImageLayout(
        image.ImageData,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal);

    return image;
}

void HelloTriangle::transitionImageLayout(
    vk::raii::Image& image,
    vk::Format       format,
    vk::ImageLayout  oldLayout,
    vk::ImageLayout  newLayout)
{
    vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier(
        {},
        {},
        oldLayout,
        newLayout,
        {},
        {},
        *image,
        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage      = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    std::array<vk::MemoryBarrier, 0>       memoryBarrier;
    std::array<vk::BufferMemoryBarrier, 0> bufferMemoryBarier;
    std::array<vk::ImageMemoryBarrier, 1>  imageMemoryBarrier({barrier});

    commandBuffer
        .pipelineBarrier(sourceStage, destinationStage, {}, memoryBarrier, bufferMemoryBarier, imageMemoryBarrier);

    endSingleTimeCommands(commandBuffer);
}

HelloTriangle::BufferStruct HelloTriangle::createVertexBuffer()
{
    size_t memorySize = sizeof(m_Vertices[0]) * m_Vertices.size();

    auto buffer =
        createBuffer(memorySize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
    auto memory =
        createDeviceMemory(vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible, buffer);
    BufferStruct vertexBuffer(std::move(buffer), std::move(memory));

    vk::raii::Buffer       stagingBuffer       = createBuffer(memorySize, vk::BufferUsageFlagBits::eTransferSrc);
    vk::raii::DeviceMemory stagingBufferMemory = createDeviceMemory(
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
        stagingBuffer);

    uint8_t* pData = static_cast<uint8_t*>(stagingBufferMemory.mapMemory(0, memorySize));
    memcpy(pData, m_Vertices.data(), memorySize);
    stagingBufferMemory.unmapMemory();

    copyBuffer(stagingBuffer, vertexBuffer.BufferData, memorySize);

    return vertexBuffer;
}

HelloTriangle::BufferStruct HelloTriangle::createIndexBuffer()
{
    size_t memorySize = sizeof(m_Indeces[0]) * m_Indeces.size();
    auto   buffer =
        createBuffer(memorySize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
    auto memory =
        createDeviceMemory(vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible, buffer);

    BufferStruct indexBuffer(std::move(buffer), std::move(memory));

    vk::raii::Buffer       stagingBuffer       = createBuffer(memorySize, vk::BufferUsageFlagBits::eTransferSrc);
    vk::raii::DeviceMemory stagingBufferMemory = createDeviceMemory(
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
        stagingBuffer);

    uint8_t* pData = static_cast<uint8_t*>(stagingBufferMemory.mapMemory(0, memorySize));
    memcpy(pData, m_Indeces.data(), memorySize);
    stagingBufferMemory.unmapMemory();

    copyBuffer(stagingBuffer, indexBuffer.BufferData, memorySize);

    return indexBuffer;
}

std::vector<HelloTriangle::BufferStruct> HelloTriangle::createUniformBuffers()
{
    vk::DeviceSize memorySize = sizeof(UniformBufferObject);

    std::vector<BufferStruct> buffers;
    int                       swapchainImageSize = static_cast<int>(m_SwapChainImages.size());

    for (int i = 0; i < swapchainImageSize; ++i)
    {
        auto buffer = createBuffer(memorySize, vk::BufferUsageFlagBits::eUniformBuffer);
        auto memory = createDeviceMemory(
            vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
            buffer);

        buffers.push_back(BufferStruct(std::move(buffer), std::move(memory)));
    }
    return buffers;
}

vk::raii::DescriptorPool HelloTriangle::createDescriptorPool()
{
    vk::DescriptorPoolSize poolSize0(
        vk::DescriptorType::eUniformBuffer,
        static_cast<uint32_t>(m_SwapChainImages.size()));

    vk::DescriptorPoolSize poolSize1(
        vk::DescriptorType::eCombinedImageSampler,
        static_cast<uint32_t>(m_SwapChainImages.size()));

    std::array<vk::DescriptorPoolSize, 2> poolSizes({std::move(poolSize0), std::move(poolSize1)});

    vk::DescriptorPoolCreateInfo createInfo(
        vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        static_cast<uint32_t>(m_SwapChainImages.size()),
        poolSizes);

    return vk::raii::DescriptorPool(m_Device, createInfo, nullptr);
}

vk::raii::DeviceMemory HelloTriangle::createDeviceMemory(
    vk::MemoryPropertyFlags propertyFlags,
    vk::raii::Buffer&       bindBuffer)
{
    // allocate device memory for that buffer
    vk::MemoryRequirements memoryRequirements = bindBuffer.getMemoryRequirements();

    uint32_t memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);

    vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);
    vk::raii::DeviceMemory deviceMemory(m_Device, memoryAllocateInfo);
    // and bind the device memory to the vertex buffer
    bindBuffer.bindMemory(*deviceMemory, 0);

    return deviceMemory;
}

vk::raii::DescriptorSets HelloTriangle::createDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts;
    for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
    {
        layouts.push_back(*m_DescriptorLayout);
    }
    vk::DescriptorSetAllocateInfo allocInfo(*m_DescriptorPool, m_SwapChainImages.size(), layouts.data());

    vk::raii::DescriptorSets descriptorSets(m_Device, allocInfo);

    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        vk::DescriptorBufferInfo bufferInfo(*(m_UniformBuffers[i].BufferData), 0, sizeof(UniformBufferObject));

        vk::DescriptorImageInfo imageInfo(
            *m_TextureSampler,
            *m_TextureImageView,
            vk::ImageLayout::eShaderReadOnlyOptimal);

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites;
        descriptorWrites.at(0) = vk::WriteDescriptorSet(
            *descriptorSets[i],
            0,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            &bufferInfo,
            nullptr);

        descriptorWrites.at(1) = vk::WriteDescriptorSet(
            *descriptorSets[i],
            1,
            0,
            1,
            vk::DescriptorType::eCombinedImageSampler,
            &imageInfo,
            nullptr,
            nullptr);

        std::array<vk::CopyDescriptorSet, 0> copyDescriptorSets;
        m_Device.updateDescriptorSets(descriptorWrites, copyDescriptorSets);
    }

    return std::move(descriptorSets);
}

vk::raii::Sampler HelloTriangle::createTextureSampler()
{
    vk::SamplerCreateInfo createInfo(
        {},
        vk::Filter::eLinear,
        vk::Filter::eLinear,
        vk::SamplerMipmapMode::eLinear,
        vk::SamplerAddressMode::eRepeat,
        vk::SamplerAddressMode::eRepeat,
        vk::SamplerAddressMode::eRepeat,
        0.0f,
        VK_TRUE,
        m_PhysicalDevice.getProperties().limits.maxSamplerAnisotropy,
        VK_FALSE,
        vk::CompareOp::eAlways,
        0.0f,
        0.0f,
        vk::BorderColor::eIntOpaqueBlack,
        VK_FALSE);

    return vk::raii::Sampler(m_Device, createInfo);
}

vk::raii::ImageView HelloTriangle::createImageView(vk::Image image, vk::Format format)
{
    vk::ImageViewCreateInfo viewInfo(
        {},
        image,
        vk::ImageViewType::e2D,
        format,
        {},
        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    return vk::raii::ImageView(m_Device, viewInfo);
}

void HelloTriangle::copyBufferToImage(vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height)
{
    vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();

    std::array<vk::BufferImageCopy, 1> regions({vk::BufferImageCopy(
        0,
        0,
        0,
        vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
        vk::Offset3D(0, 0, 0),
        vk::Extent3D(width, height, 1))});

    commandBuffer.copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal, regions);

    endSingleTimeCommands(commandBuffer);
}

void HelloTriangle::copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size)
{
    vk::raii::CommandBuffer       commandBuffer = beginSingleTimeCommands();
    std::array<vk::BufferCopy, 1> copyRegion    = {vk::BufferCopy(0, 0, size)};
    commandBuffer.copyBuffer(*srcBuffer, *dstBuffer, copyRegion);

    endSingleTimeCommands(commandBuffer);
}

vk::raii::CommandBuffer HelloTriangle::beginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo(*m_CommandPool, vk::CommandBufferLevel::ePrimary, 1);
    vk::raii::CommandBuffer       commandBuffer = std::move(vk::raii::CommandBuffers(m_Device, allocInfo).front());

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void HelloTriangle::endSingleTimeCommands(vk::raii::CommandBuffer& buffer)
{
    buffer.end();

    auto buffers = std::array<vk::CommandBuffer, 1>({*buffer});

    vk::SubmitInfo submitInfo({}, {}, buffers);

    m_GraphicsQueue.submit(std::array<vk::SubmitInfo, 1>({submitInfo}));
    m_GraphicsQueue.waitIdle();
}

void HelloTriangle::drawFrame()
{
    if (vk::Result::eSuccess != m_Device.waitForFences(*(m_InFlightFences[m_CurrentFrame]), VK_TRUE, UINT64_MAX))
    {
        throw std::runtime_error("Failed waiting for fence to clear.");
    }

    vk::Result result;
    uint32_t   imageIndex;

    std::tie(result, imageIndex) =
        m_SwapChain.acquireNextImage(UINT64_MAX, *(m_ImageAvailableSemaphores[m_CurrentFrame]));

    if (imageIndex >= m_ImageViews.size())
    {
        throw std::runtime_error("Failed to get next image.");
    }

    if (m_ImagesInFlight.at(imageIndex))
    {
        if (vk::Result::eSuccess != m_Device.waitForFences(*(*m_ImagesInFlight[imageIndex]), VK_TRUE, UINT64_MAX))
        {
            throw std::runtime_error("Failed waiting for fence to clear.");
        }
    }
    m_ImagesInFlight[imageIndex] = &m_InFlightFences[m_CurrentFrame];

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

    updateUniformBuffer(imageIndex);

    vk::SubmitInfo submitInfo(
        *(m_ImageAvailableSemaphores[m_CurrentFrame]),
        waitDestinationStageMask,
        *(m_CommandBuffers[imageIndex]),
        *(m_RenderFinishedSemaphores[m_CurrentFrame]));

    m_Device.resetFences(*(m_InFlightFences[m_CurrentFrame]));

    m_GraphicsQueue.submit(submitInfo, *(m_InFlightFences[m_CurrentFrame]));

    vk::PresentInfoKHR presentInfoKHR(*(m_RenderFinishedSemaphores[m_CurrentFrame]), *m_SwapChain, imageIndex);
    result = m_PresentQueue.presentKHR(presentInfoKHR);

    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        m_FramebufferResized = false;
        recreateSwapChain();
        return;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
void HelloTriangle::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto  currentTime = std::chrono::high_resolution_clock::now();
    float time        = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj =
        glm::perspective(glm::radians(45.0f), m_SwapChainExtents.width / (float)m_SwapChainExtents.height, 0.1f, 10.0f);

    ubo.proj[1][1] *= -1;

    uint8_t* pData = static_cast<uint8_t*>(m_UniformBuffers[currentImage].BufferMemory.mapMemory(0, sizeof(ubo)));
    memcpy(pData, &ubo, sizeof(ubo));
    m_UniformBuffers[currentImage].BufferMemory.unmapMemory();
}

void HelloTriangle::recreateSwapChain()
{
    auto size = m_Window.GetFrameBufferSize();
    while (size.Width == 0 || size.Height == 0)
    {
        size = m_Window.GetFrameBufferSize();
        glfwWaitEvents();
    }

    m_Device.waitIdle();

    m_SwapChain      = createSwapChain(*m_SwapChain);
    m_ImageViews     = createImageViews();
    m_RenderPass     = createRenderPass();
    m_Pipeline       = createPipeline();
    m_Framebuffers   = createFrameBuffers();
    m_UniformBuffers = createUniformBuffers();

    m_DescriptorSets.clear();
    m_DescriptorPool = createDescriptorPool();
    m_DescriptorSets = createDescriptorSets();
    m_CommandBuffers = createCommandBuffers();

    for (auto& fence : m_ImagesInFlight)
    {
        fence = nullptr;
    }
}

void HelloTriangle::waitImagesInFlight()
{
    for (size_t i = 0; i < m_ImagesInFlight.size(); ++i)
    {
        if (m_ImagesInFlight.at(i))
        {
            if (vk::Result::eSuccess != m_Device.waitForFences(*(*m_ImagesInFlight[i]), VK_TRUE, UINT64_MAX))
            {
                throw std::runtime_error("Failed waiting for fence to clear.");
            }
        }

        m_ImagesInFlight.at(i) = nullptr;
    }
}
void HelloTriangle::onWindowResize(int width, int height)
{
    m_FramebufferResized = true;
    m_NewHeight          = height;
    m_NewWidth           = width;
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

uint32_t HelloTriangle::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = m_PhysicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
