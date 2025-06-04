#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

class VertexArray
{
public:
    VertexArray() = default;
    VertexArray(const VkDevice& device,
                const VkPhysicalDevice& physical_device,
                uint32_t binding,
                const std::vector<uint32_t>& queue_families,
                const std::vector<Vertex>& vertices);
    ~VertexArray() = default;

    VkVertexInputBindingDescription GetBindingDescription() const;
    std::array<VkVertexInputAttributeDescription, 2> GetAttribDescriptions() const;
    const VkBuffer& GetVertexBuffer() const;
    void Deallocate() const;

private:
    VkDevice m_device;
    uint32_t m_binding;
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_bufferMemory;

private:
    MLC_NODISCARD uint32_t _FindMemoryType(const VkPhysicalDevice& physical_device,
                                           uint32_t type_filter,
                                           VkMemoryPropertyFlags properties);
};

MLC_NAMESPACE_END