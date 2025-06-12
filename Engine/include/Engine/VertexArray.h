#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "Engine/core/Defines.h"
#include "Engine/GPUBuffer.h"
#include "Engine/VulkanManager.h"

MLC_NAMESPACE_START

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

class VertexArray
{
public:
    VertexArray(const VulkanManager* vulkan_manager,
                uint32_t binding,
                const std::vector<Vertex>& vertices);
    ~VertexArray();
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    MLC_NODISCARD VkVertexInputBindingDescription GetBindingDescription() const;
    MLC_NODISCARD std::array<VkVertexInputAttributeDescription, 2> GetAttribDescriptions() const;
    MLC_NODISCARD uint32_t GetVerticesCount() const;
    MLC_NODISCARD const GPUBuffer& GetGPUBuffer() const;

private:
    const VulkanManager* m_vulkanManager;
    uint32_t m_binding = static_cast<uint32_t>(-1);
    uint32_t m_verticesCount = static_cast<uint32_t>(-1);
    GPUBuffer m_buffer;

private:
    MLC_NODISCARD uint32_t _FindMemoryType(const VkPhysicalDevice& physical_device,
                                           uint32_t type_filter,
                                           VkMemoryPropertyFlags properties);
};

MLC_NAMESPACE_END