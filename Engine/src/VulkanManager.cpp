#include "Engine/VulkanManager.h"

#include <unordered_map>
#include <algorithm>
#include <array>
#include <set>
#include <filesystem>

#include "Engine/core/Assert.h"
#include "Engine/core/Debug.h"
#include "Engine/core/Logging.h"
#include "Engine/Shader.h"

MLC_NAMESPACE_START

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

const std::array<const char*, 1> VALIDATION_LAYERS {
    "VK_LAYER_KHRONOS_validation"  // common validation layers bundled into one
};

const std::array<const char*, 1> DEVICE_EXTENSIONS {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static std::unordered_map<std::string_view, bool> s_supportedExtensions;
static std::unordered_map<std::string_view, bool> s_supportedLayers;

void VulkanManager::Init(GLFWwindow* window)
{
    // Note: Every vkCreateXXX has a mandatory vkDestroyXXX
    //       vkGetXXX may be implicitly destroyed
    m_window = window;

    if (enableValidationLayers)
    {
        MLC_ASSERT(_CheckValidationLayerSupport(), "Validation Layer enabled, not available.");
    }
    _CreateInstance();
    _SetupDebugMessenger();
    _CreateSurface();
    _PickPhysicalDevice();
    _CreateLogicalDevice();
    _GetQueues();
    _CreateSwapChain();
    _CreateImageViews();
    _CreateRenderPass();
    _CreateGraphicsPipeline();

    MLC_INFO("Vulkan Initialization: Success");
}

void VulkanManager::ShutDown()
{
    vkDestroyPipeline(m_device, m_graphicsPipeline, MLC_VULKAN_ALLOCATOR);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, MLC_VULKAN_ALLOCATOR);
    vkDestroyRenderPass(m_device, m_renderPass, MLC_VULKAN_ALLOCATOR);
    for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
    {
        vkDestroyImageView(m_device, m_swapChainImageViews[i], MLC_VULKAN_ALLOCATOR);
    }
    vkDestroySwapchainKHR(m_device, m_swapChain, MLC_VULKAN_ALLOCATOR);
    vkDestroyDevice(m_device, MLC_VULKAN_ALLOCATOR);
    vkDestroySurfaceKHR(m_instance, m_surface, MLC_VULKAN_ALLOCATOR);
    if (enableValidationLayers)
    {
        _DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, MLC_VULKAN_ALLOCATOR);
    }
    vkDestroyInstance(m_instance, MLC_VULKAN_ALLOCATOR);

    MLC_INFO("Vulkan Deinitialization: Success");
}

MLC_NODISCARD std::vector<const char*> VulkanManager::_GLFWGetRequiredExtensions()
{
    uint32_t glfwExtensionCount;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    MLC_ASSERT(glfwExtensions != NULL, "Failed to get required extensions.");

    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    if (enableValidationLayers)
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return requiredExtensions;
}

MLC_NODISCARD bool VulkanManager::_CheckValidationLayerSupport()
{
    for (uint32_t i = 0; i < VALIDATION_LAYERS.size(); i++)
    {
        s_supportedLayers[VALIDATION_LAYERS[i]] = true;
    }

    uint32_t supportedLayerCount;
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);
    std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
    vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data());

    for (const char* validation_layer : VALIDATION_LAYERS)
    {
        if (!s_supportedLayers[validation_layer]) return false;
    }
    return true;
}

void VulkanManager::_CreateInstance()
{
    VkApplicationInfo vkApplicationInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Malic",
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = "Malic Engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    std::vector<const char*> requiredExtensions = _GLFWGetRequiredExtensions();

    uint32_t supportedExtensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
    std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

    fmt::print("{} supported Vulkan extensions.\n", supportedExtensionCount);
    for (const VkExtensionProperties& extension : supportedExtensions)
    {
        // fmt::print("{} [{}]\n", strlen(extension.extensionName), extension.specVersion);
        s_supportedExtensions[extension.extensionName] = true;
    }
    fmt::print("Required Vulkan extensions ({}): \n", requiredExtensions.size());
    for (uint32_t i = 0; i < requiredExtensions.size(); i++)
    {
        fmt::print("    {} [{}]\n",
            requiredExtensions[i],
            s_supportedExtensions[requiredExtensions[i]] ? "SUPPORTED" : "UNSUPPORTED"
        );
    }

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VulkanDebugCallback,
        .pUserData = nullptr
    };

    VkInstanceCreateInfo vkInstanceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugUtilsMessengerCreateInfo,
        .flags = 0,
        .pApplicationInfo = enableValidationLayers ? &vkApplicationInfo : nullptr,
        .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(VALIDATION_LAYERS.size()) : 0, 
        .ppEnabledLayerNames = enableValidationLayers ? VALIDATION_LAYERS.data() : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };

    VkResult result = vkCreateInstance(&vkInstanceCreateInfo, MLC_VULKAN_ALLOCATOR, &m_instance);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create VkInstance.");
}

VkResult VulkanManager::_CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == nullptr)
        return VK_ERROR_EXTENSION_NOT_PRESENT;

    return func(instance, pCreateInfo, pAllocator, pMessenger);
}

void VulkanManager::_DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
    MLC_ASSERT(func != nullptr, "Failed to delete Debug Messenger.");

    func(instance, debugMessenger, pAllocator);
}

void VulkanManager::_CreateSurface()
{
    // VkWin32SurfaceCreateInfoKHR surfaceCreateInfo {
    //     .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    //     .pNext = nullptr,
    //     .flags = 0,
    //     .hinstance = GetModuleHandle(nullptr),
    //     .hwnd = glfwGetWin32Window(glfwGetCurrentContext())
    // };

    // VkResult result = vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, MLC_VULKAN_ALLOCATOR, &m_surface);
    // MLC_ASSERT(result == VK_SUCCESS, "Failed to create window surface.");
    
    VkResult result = glfwCreateWindowSurface(m_instance, m_window, MLC_VULKAN_ALLOCATOR, &m_surface);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create window surface.");
}

void VulkanManager::_SetupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = VulkanDebugCallback,
        .pUserData = nullptr
    };
    VkResult result = _CreateDebugUtilsMessengerEXT(m_instance,
                                                    &debugMessengerCreateInfo,
                                                    MLC_VULKAN_ALLOCATOR,
                                                    &m_debugMessenger);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to set up debug messenger.");
}

QueueFamiliesIndices VulkanManager::_FindQueueFamilies(const VkPhysicalDevice& device)
{
    QueueFamiliesIndices familyIndices;
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (int i = 0; i < queueFamilyCount; i++)
    {
        uint32_t thisQueueCount = 0;
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && thisQueueCount < queueFamilies[i].queueCount)
        {
            familyIndices.graphicsFamily = i;
            // thisQueueCount++;
        }
        if (presentSupport && thisQueueCount < queueFamilies[i].queueCount)  // might be the same queue family as graphics queues
        {
            familyIndices.presentFamily = i;
        }

        // Found all required queue families
        if (familyIndices.IsComplete()) break;
    }

    return familyIndices;
}

bool VulkanManager::_CheckDeviceExtensionSupport(const VkPhysicalDevice& physical_device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string_view> requiredDeviceExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
    for (const VkExtensionProperties& extension : availableExtensions)
    {
        requiredDeviceExtensions.erase(extension.extensionName);
    }

    return requiredDeviceExtensions.empty();
}

SwapChainSupportDetails VulkanManager::_QuerySwapChainSupport(const VkPhysicalDevice& physical_device)
{
    SwapChainSupportDetails details;

    // Query capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, m_surface, &details.capabilities);

    // Query formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &formatCount, nullptr);
    MLC_ASSERT(formatCount != 0, "Failed to query for pixel format count.");
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface, &formatCount, details.formats.data());

    // Query present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &presentModeCount, nullptr);
    MLC_ASSERT(presentModeCount != 0, "Failed to query for present mode count.");
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface, &presentModeCount, details.presentModes.data());

    return details;
}

bool VulkanManager::_IsPhysicalDeviceSuitable(const VkPhysicalDevice& physical_device)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceProperties(physical_device, &physicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(physical_device, &physicalDeviceFeatures);
    fmt::print("{}\n", physicalDeviceProperties.deviceName);

    // Each time _IsPhysicalDeviceSuitable is run, m_queueFamilyIndices will be
    // set to another set of queue familes.
    // When the function returns true, it's always the suitable set.
    m_queueFamilyIndices = _FindQueueFamilies(physical_device);

    bool extensionsSupported = _CheckDeviceExtensionSupport(physical_device);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupportDetails = _QuerySwapChainSupport(physical_device);
        swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
    }

    return m_queueFamilyIndices.IsComplete() && extensionsSupported && swapChainAdequate;
}

void VulkanManager::_PickPhysicalDevice()
{
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    MLC_ASSERT(deviceCount > 0, "Failed to find GPU with Vulkan support.");
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());

    for (VkPhysicalDevice& physical_device : physicalDevices)
    {
        if (_IsPhysicalDeviceSuitable(physical_device))
        {
            m_physicalDevice = physical_device;
            break;
        }
    }

    MLC_ASSERT(m_physicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU.");
}

void VulkanManager::_CreateLogicalDevice()
{
    // Queue create info and device features
    float queuePriority = 1.0f;

    // Repeat values are deduplicated
    const std::set<uint32_t> uniqueQueueFamilies {
        m_queueFamilyIndices.graphicsFamily.value(),
        m_queueFamilyIndices.presentFamily.value()
    };

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t queue_family : uniqueQueueFamilies)
    {
        queueCreateInfos.push_back(VkDeviceQueueCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queue_family,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        });
    }

    VkPhysicalDeviceFeatures deviceFeatures {};

    // Device creation here
    VkDeviceCreateInfo deviceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(VALIDATION_LAYERS.size()) : 0,
        .ppEnabledLayerNames = enableValidationLayers ? VALIDATION_LAYERS.data() : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size()),
        .ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(),
        .pEnabledFeatures = &deviceFeatures
    };

    // Create logical device
    VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, MLC_VULKAN_ALLOCATOR, &m_device);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create logical device.");

    // Note: Get queues in main Init function
}

void VulkanManager::_GetQueues()
{
    // If the graphics and present queues are the same, they'll have the same address
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);
}

VkExtent2D VulkanManager::_ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        VkExtent2D actualExtent {
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width,
                                        capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

VkSurfaceFormatKHR VulkanManager::_ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    for (const VkSurfaceFormatKHR& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats[0];  // could be chosen based on given assessed rank
}

VkPresentModeKHR VulkanManager::_ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& present_modes)
{
    for (const VkPresentModeKHR& present_mode : present_modes)
    {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return present_mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void VulkanManager::_CreateSwapChain()
{
    VkExtent2D extent;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    uint32_t imageCount;  // how many images in swap chain
    std::array<uint32_t, 2> queueFamilyIndices {
        m_queueFamilyIndices.graphicsFamily.value(),
        m_queueFamilyIndices.presentFamily.value()
    };
    bool graphicsQueueIsPresentQueue = queueFamilyIndices[0] == queueFamilyIndices[1];    

    SwapChainSupportDetails swapChainSupport = _QuerySwapChainSupport(m_physicalDevice);
    extent = _ChooseSwapChainExtent(swapChainSupport.capabilities);
    surfaceFormat = _ChooseSwapChainSurfaceFormat(swapChainSupport.formats);
    presentMode = _ChooseSwapChainPresentMode(swapChainSupport.presentModes);

    imageCount = swapChainSupport.capabilities.minImageCount + 1;  // +1 to ensure available images
                                                                   // to render to
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount)  // if maxImageCount = 0
    {                                                              // there are no limits
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Swap chain creation here
    VkSwapchainCreateInfoKHR swapChainCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = m_surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  // TODO: VK_IMAGE_USAGE_TRANSFER_DST_BIT for post-processing
        .imageSharingMode = graphicsQueueIsPresentQueue ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = graphicsQueueIsPresentQueue ? 0u : 2u,
        .pQueueFamilyIndices = queueFamilyIndices.data(),
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,  // WINDOW opacity, which is interesting
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    VkResult result = vkCreateSwapchainKHR(m_device, &swapChainCreateInfo, MLC_VULKAN_ALLOCATOR, &m_swapChain);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create swap chain.");

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainImageCount, nullptr);
    m_swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainImageCount, m_swapChainImages.data());
}

void VulkanManager::_CreateImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for (uint32_t i = 0; i < m_swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = m_swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_swapChainImageFormat,
            .components = VkComponentMapping {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        VkResult result = vkCreateImageView(m_device,
                                            &imageViewCreateInfo,
                                            MLC_VULKAN_ALLOCATOR,
                                            &m_swapChainImageViews[i]);
        MLC_ASSERT(result == VK_SUCCESS, "Failed to create swap chain image views.");
    }
}

void VulkanManager::_CreateRenderPass()
{
    VkAttachmentDescription colorAttachment {
        .flags = 0,
        .format = m_swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef {
        .attachment = 0,  // attachment index in array
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr
    };

    VkRenderPassCreateInfo renderPassCreateInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0,
        .pDependencies = nullptr
    };

    VkResult result = vkCreateRenderPass(m_device,
                                         &renderPassCreateInfo,
                                         MLC_VULKAN_ALLOCATOR,
                                         &m_renderPass);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create render pass.");                                        
}

void VulkanManager::_CreateGraphicsPipeline()
{
    // ----- Programmable stages of the pipeline -----
    
    std::filesystem::path vertPath = std::filesystem::path(MLC_ROOT_DIR) / "Engine/resources/shaders/bin/default_vert.spv";
    std::filesystem::path fragPath = std::filesystem::path(MLC_ROOT_DIR) / "Engine/resources/shaders/bin/default_frag.spv";
    Shader defaultShader(
        vertPath.string(),
        fragPath.string(),
        m_device
    );

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = defaultShader.vertShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr  // this can specify constants inside the shader
    };

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = defaultShader.fragShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr  // this can specify constants inside the shader
    };

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        vertShaderStageCreateInfo,
        fragShaderStageCreateInfo
    };

    // ----- Fixed stages -----

    // Dynamic states
    std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .scissorCount = 1
    };

    // Vertex input
    // VkVertexInputBindingDescription
    // VkVertexInputAttributeDescription
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr
    };

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    // Viewport & scissor
    VkViewport viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_swapChainExtent.width),
        .height = static_cast<float>(m_swapChainExtent.height),
        .minDepth = 0.0f,  // just normalized depth values (?)
        .maxDepth = 1.0f
    };
    VkRect2D scissor {
        .offset = { 0, 0 },
        .extent = m_swapChainExtent
    };

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,  // VK_TRUE might be useful for shadow maps
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,  // depth bias may be useful for shadow maps
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    // Depth buffer (later, TODO)
    // Stencil buffer (later, TODO)

    // Color blending (TODO)
    VkPipelineColorBlendAttachmentState colorBlendAttachment {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                          VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,  // Optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }  // Optional
    };

    // ----- Pipeline Layout -----

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };
    VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, MLC_VULKAN_ALLOCATOR, &m_pipelineLayout);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create pipeline layout.");

    // ----- Graphics Pipeline -----

    VkGraphicsPipelineCreateInfo pipelineCreateInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizerCreateInfo,
        .pMultisampleState = &multisamplingCreateInfo,
        .pDepthStencilState = nullptr,  // Optional
        .pColorBlendState = &colorBlendingCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = m_pipelineLayout,
        .renderPass = m_renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };
    // See https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Conclusion
    // for the last two parameters
    result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, MLC_VULKAN_ALLOCATOR, &m_graphicsPipeline);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create graphics pipeline.");
}

MLC_NAMESPACE_END