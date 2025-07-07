#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

class GPUBuffer
{
friend class VulkanManager;
public:
    GPUBuffer() = default;
    ~GPUBuffer();
    GPUBuffer(const GPUBuffer&) = delete;
    GPUBuffer& operator=(const GPUBuffer&) = delete;
    GPUBuffer(GPUBuffer&& other) noexcept;
    GPUBuffer& operator=(GPUBuffer&& other) noexcept;

    MLC_NODISCARD bool IsUsable() const;

private:
    VkBuffer m_handle = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkMemoryPropertyFlags m_properties = 0;
};

MLC_NAMESPACE_END