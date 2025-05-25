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

MLC_NAMESPACE_START

struct QueueFamiliesIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
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

class VulkanManager
{
public:
    void Init(GLFWwindow* window);
    void ShutDown();

    void WaitAndResetFence();
    void Present();
    void WaitIdle();

    VulkanManager() = default;
    ~VulkanManager() = default;

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;  // implicitly destroyed
    QueueFamiliesIndices m_queueFamilyIndices;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;  // implicitly destroyed with with VkDevice
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkSwapchainKHR m_swapChain;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImage> m_swapChainImages;  // automatically destroyed with the swapchain
    std::vector<VkImageView> m_swapChainImageViews;

    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;

    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;

    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_inFlightFence;

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

    // Create "PipelineSettings" struct and pass everything as an argument
    void _CreateGraphicsPipeline();

    void _CreateFramebuffers();

    void _CreateCommandPool();
    void _CreateCommandBuffer();
    void _RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t swch_image_index);

    void _CreateSyncObjects();
};

MLC_NAMESPACE_END