#pragma once

#include <vector>
#include <optional>
// #include <memory>

// TODO: Add Linux
#ifdef __WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Engine/core/Defines.h"
#include "Engine/GPUBuffer.h"

MLC_NAMESPACE_START

struct QueueFamiliesIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    bool IsComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
    }

    bool IsExclusive() const
    {
        return graphicsFamily.value() == presentFamily.value()
            && graphicsFamily.value() == transferFamily.value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    // capabilities: stuff like image dimensions are swap chain size
    // formats: pixel formats, color space
    // presentModes: available present modes
};

class VertexArray;
class VulkanManager
{
public:
    VulkanManager() = default;
    ~VulkanManager() = default;
    VulkanManager(const VulkanManager&) = delete;
    VulkanManager& operator=(const VulkanManager&) = delete;

    void Init(GLFWwindow* window);
    void ShutDown();

    void Present(const std::vector<VertexArray>& vertex_arrays);
    void WaitIdle();
    void ResizeFramebuffer();
    
    void AllocateBuffer(GPUBuffer& buffer,
                        VkDeviceSize size,
                        VkBufferUsageFlags usage,
                        VkMemoryPropertyFlags properties) const;
    void DeallocateBuffer(GPUBuffer& buffer) const;
    void UploadBuffer(const GPUBuffer& buffer, const void* data, size_t size) const;
    void CopyBuffer(const GPUBuffer& src, const GPUBuffer& dst, VkDeviceSize size) const;
    void CreateFences(VkFence* fences, uint32_t amount, bool signaled = false) const;
    void DestroyFences(VkFence* fences, uint32_t amount) const;

    // Create "PipelineSettings" struct and pass everything as an argument
    void CreateGraphicsPipeline(const std::vector<VertexArray>& vertex_arrays);

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;  // implicitly destroyed
    QueueFamiliesIndices m_queueFamilyIndices;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;  // implicitly destroyed with with VkDevice
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkQueue m_transferQueue = VK_NULL_HANDLE;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImage> m_swapChainImages;  // automatically destroyed with the swapchain
    std::vector<VkImageView> m_swapChainImageViews;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkCommandPool m_graphicsCmdPool = VK_NULL_HANDLE;
    VkCommandPool m_transferCmdPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_graphicsCmdBuffers;
    std::vector<VkCommandBuffer> m_transferCmdBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    bool m_framebufferResized = false;
    GLFWwindow* m_window;

private:
    MLC_NODISCARD std::vector<const char*> _GLFWGetRequiredExtensions();
    MLC_NODISCARD bool _CheckValidationLayerSupport();
    void _CreateInstance();

    VkResult _CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pMessenger
    );
    void _DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
    );
    void _SetupDebugMessenger();

    void _CreateSurface();
    
    MLC_NODISCARD QueueFamiliesIndices _FindQueueFamilies(const VkPhysicalDevice& device);
    MLC_NODISCARD bool _CheckDeviceExtensionSupport(const VkPhysicalDevice& physical_device);
    MLC_NODISCARD SwapChainSupportDetails _QuerySwapChainSupport(const VkPhysicalDevice& physical_device);
    bool _IsPhysicalDeviceSuitable(const VkPhysicalDevice& physical_device);
    void _PickPhysicalDevice();
    
    void _CreateLogicalDevice();

    void _GetQueues();
    
    MLC_NODISCARD VkExtent2D _ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    MLC_NODISCARD VkSurfaceFormatKHR _ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    MLC_NODISCARD VkPresentModeKHR _ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& present_modes);
    void _CreateSwapChain();
    void _CreateImageViews();

    void _CreateRenderPass();

    void _CreateFramebuffers();

    void _CreateCommandPools();
    void _CreateCommandBuffers();
    void _RecordCommandBuffer(VkCommandBuffer command_buffer,
                              uint32_t swch_image_index,
                              const std::vector<VertexArray>& vertex_arrays);

    void _CreateSyncObjects();

    // Window resize events lead to swap chain recreation
    void _RecreateSwapChain();
    
    uint32_t _FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
};

MLC_NAMESPACE_END