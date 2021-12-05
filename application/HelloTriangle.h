#pragma once

#include "GLFWWindow.h"
#include "Shader.h"
#include "VulkanDebugLog.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <array>
#include <optional>
#include <string>

class HelloTriangle {
    struct BufferStruct
    {
        BufferStruct(vk::raii::Buffer buffer, vk::raii::DeviceMemory memory)
            : BufferData(std::move(buffer))
            , BufferMemory(std::move(memory))
        {
        }

        vk::raii::Buffer       BufferData;
        vk::raii::DeviceMemory BufferMemory;
    };

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

    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;

        static std::array<vk::VertexInputBindingDescription, 1>   getBindingDescription();
        static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
    };

#ifdef NDEBUG
    static constexpr bool ENABLE_VALIDATION = false;
#else
    static constexpr bool ENABLE_VALIDATION = true;
#endif
    static constexpr int     MAX_FRAMES_IN_FLIGHT = 2;
    static const std::string AppName;
    static const std::string EngineName;

    bool                  m_EnablePortabilityExtension = false;
    std::vector<Vertex>   m_Vertices;
    std::vector<uint16_t> m_Indeces;

    GLFWWindow m_Window;

    vk::raii::Context    m_Context;
    vk::raii::Instance   m_Instance;
    vk::raii::SurfaceKHR m_Surface;

#ifndef NDEBUG
    VulkanDebugLog m_Logger;
#endif

    vk::raii::PhysicalDevice m_PhysicalDevice;
    vk::raii::Device         m_Device;
    vk::raii::Queue          m_GraphicsQueue;
    vk::raii::Queue          m_PresentQueue;

    vk::SurfaceFormatKHR             m_SwapChainImageFormat;
    vk::Extent2D                     m_SwapChainExtents;
    vk::raii::SwapchainKHR           m_SwapChain;
    std::vector<vk::raii::ImageView> m_ImageViews;

    vk::raii::RenderPass     m_RenderPass;
    vk::raii::PipelineLayout m_PipelineLayout;
    vk::raii::Pipeline       m_Pipeline;

    std::vector<vk::raii::Framebuffer> m_Framebuffers;
    vk::raii::CommandPool              m_CommandPool;
    BufferStruct                       m_VertexBuffer;
    BufferStruct                       m_IndexBuffer;
    vk::raii::CommandBuffers           m_CommandBuffers;

    std::vector<vk::raii::Semaphore> m_ImageAvailableSemaphores;
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::raii::Fence>     m_InFlightFences;
    std::vector<vk::raii::Fence*>    m_ImagesInFlight;

    size_t m_CurrentFrame;
    bool   m_FramebufferResized;
    int    m_NewWidth;
    int    m_NewHeight;

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
    vk::raii::Instance                 createInstance(const vk::raii::Context& context, const GLFWWindow& window);
    QueueFamilyIndices                 getQueueFamilyIndeces(const vk::raii::PhysicalDevice& physicalDevice);
    vk::raii::PhysicalDevice           getPhysicalDevice(const vk::raii::Instance& instance);
    vk::raii::Device                   createDevice();
    SwapChainSupportDetails            getSwapChainSupportDetails();
    vk::raii::SwapchainKHR             createSwapChain();
    std::vector<vk::raii::ImageView>   createImageViews();
    vk::raii::RenderPass               createRenderPass();
    vk::raii::PipelineLayout           createPipelineLayout();
    vk::raii::Pipeline                 createPipeline();
    std::vector<vk::raii::Framebuffer> createFrameBuffers();
    vk::raii::CommandPool              createCommandPool();
    vk::raii::CommandBuffers           createCommandBuffers();
    vk::raii::Buffer                   createBuffer(vk::DeviceSize bufferSize, vk::BufferUsageFlags bufferFlags);
    vk::raii::DeviceMemory createDeviceMemory(vk::MemoryPropertyFlags propertyFlags, vk::raii::Buffer& bindBuffer);

    BufferStruct createVertexBuffer();
    BufferStruct createIndexBuffer();
    void         drawFrame();
    void         recreateSwapChain();
    void         copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);

    void         waitImagesInFlight();
    void         onWindowResize(int width, int height);
    vk::Extent2D get2DExtents(vk::SurfaceCapabilitiesKHR capabilities, GLFWWindow::FrameBufferSize frameBufferSize);
    uint32_t     findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    static vk::SurfaceFormatKHR getSwapChainFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

    static bool areDeviceExtensionsSupported(
        std::vector<const char*>        extensions,
        const vk::raii::PhysicalDevice& physicalDevice);
};