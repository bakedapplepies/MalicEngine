#include "Engine/UniformBuffer.h"

#include <cstring>

MLC_NAMESPACE_START

UniformBuffer::UniformBuffer(const VulkanManager* vulkan_manager, uint32_t binding, VkDeviceSize size)
    : m_vulkanManager(vulkan_manager)
{
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_vulkanManager->AllocateBuffer(m_buffers[i],
                                        size,
                                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        m_mappedMemories[i] = m_vulkanManager->GetBufferMapping(m_buffers[i], 0, size);
    }
    m_vulkanManager->DescriptorSetBindUBO(m_buffers, 0, size);
}

UniformBuffer::~UniformBuffer()
{
    if (m_vulkanManager)
    {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_vulkanManager->DeallocateBuffer(m_buffers[i]);
        }
    }
}

UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
{
    m_vulkanManager = other.m_vulkanManager;
    m_buffers = std::move(other.m_buffers);
    m_mappedMemories = std::move(other.m_mappedMemories);

    other.m_vulkanManager = nullptr;
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other) noexcept
{
    m_vulkanManager = other.m_vulkanManager;
    m_buffers = std::move(other.m_buffers);
    m_mappedMemories = std::move(other.m_mappedMemories);

    other.m_vulkanManager = nullptr;

    return *this;
}

void UniformBuffer::UpdateData(const void* data, VkDeviceSize size) const
{
    uint32_t currentFrameInFlight = m_vulkanManager->GetCurrentFrameInFlight();
    memcpy(m_mappedMemories[currentFrameInFlight], data, static_cast<size_t>(size));
}

MLC_NAMESPACE_END