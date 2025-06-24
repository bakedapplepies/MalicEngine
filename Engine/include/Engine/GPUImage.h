#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

class GPUImage
{
friend class VulkanManager;
public:
    GPUImage() = default;
    ~GPUImage();
    GPUImage(const GPUImage&) = delete;
    GPUImage& operator=(const GPUImage&) = delete;
    GPUImage(GPUImage&& other) noexcept;
    GPUImage& operator=(GPUImage&& other) noexcept;

    bool IsUsable() const;

private:
    VkImage m_handle = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkMemoryPropertyFlags m_properties = 0;
};

MLC_NAMESPACE_END