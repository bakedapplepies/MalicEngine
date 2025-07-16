#pragma once

#include <array>

#include "Engine/core/Defines.h"
#include "Engine/core/Config.h"
#include "Engine/VulkanManager.h"
#include "Engine/GPUBuffer.h"

MLC_NAMESPACE_START

class UniformBuffer
{
public:
    // Note: UBO's will probably have to idea about types, only the data inside the pointer (void*)
    UniformBuffer() = default;
    UniformBuffer(const VulkanManager* vulkan_manager, uint32_t binding, VkDeviceSize size);
    ~UniformBuffer();
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;
    UniformBuffer(UniformBuffer&& other) noexcept;
    UniformBuffer& operator=(UniformBuffer&& other) noexcept;

    void UpdateData(const void* data, VkDeviceSize size) const;

private:
    const VulkanManager* m_vulkanManager;
    std::array<GPUBuffer, MAX_DESCRIPTOR_SETS> m_buffers;
    std::array<void*, MAX_DESCRIPTOR_SETS> m_mappedMemories;
};

MLC_NAMESPACE_END