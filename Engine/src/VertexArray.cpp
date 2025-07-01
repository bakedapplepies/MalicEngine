#include "Engine/VertexArray.h"

#include <array>

#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

VertexArray::VertexArray(const VulkanManager* vulkan_manager,
                         uint32_t binding,
                         const std::vector<Vertex>& vertices,
                         const std::vector<uint16_t>& indices)
    : m_vulkanManager(vulkan_manager),
      m_binding(binding),
      m_verticesCount(static_cast<uint32_t>(vertices.size())),
      m_indicesCount(static_cast<uint32_t>(indices.size()))
{
    // TODO: vkBindBufferMemory2: Bind multiple buffers at once
    // vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos)

    GPUBuffer stagingBuffer;

    // Upload vertex buffer
    m_vulkanManager->AllocateBuffer(stagingBuffer,
                                    vertices.size() * sizeof(Vertex),
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);    
    m_vulkanManager->AllocateBuffer(m_vertexBuffer,
                                    vertices.size() * sizeof(Vertex),
                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_vulkanManager->UploadBuffer(stagingBuffer,
                                  vertices.data(),
                                  sizeof(Vertex) * vertices.size());
    m_vulkanManager->CopyBuffer(stagingBuffer, m_vertexBuffer, sizeof(Vertex) * vertices.size());
    m_vulkanManager->DeallocateBuffer(stagingBuffer);

    // Upload index buffer
    m_vulkanManager->AllocateBuffer(stagingBuffer,
                                    sizeof(uint16_t) * indices.size(),
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_vulkanManager->AllocateBuffer(m_indexBuffer,
                                    sizeof(uint16_t) * indices.size(),
                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_vulkanManager->UploadBuffer(stagingBuffer,
                                  indices.data(),
                                  sizeof(uint16_t) * indices.size());
    m_vulkanManager->CopyBuffer(stagingBuffer, m_indexBuffer, sizeof(uint16_t) * indices.size());
    m_vulkanManager->DeallocateBuffer(stagingBuffer);
}
                                    
VertexArray::~VertexArray()
{
    // Note: When performing move semantics, buffer handles with value VK_NULL_HANDLE
    // can still be safely called with vkDestroyBuffer, but of course it can't be
    // used with other functions, which can prevent implicitly ownership of buffers
    if (m_vulkanManager)
    {
        m_vulkanManager->DeallocateBuffer(m_vertexBuffer);
        m_vulkanManager->DeallocateBuffer(m_indexBuffer);
    }
}

VertexArray::VertexArray(VertexArray&& other) noexcept
{
    m_vulkanManager = other.m_vulkanManager;
    m_binding = other.m_binding;
    m_verticesCount = other.m_verticesCount;
    m_indicesCount = other.m_indicesCount;
    m_vertexBuffer = std::move(other.m_vertexBuffer);
    m_indexBuffer = std::move(other.m_indexBuffer);

    other.m_vulkanManager = nullptr;
    other.m_binding = static_cast<uint32_t>(-1);
    other.m_verticesCount = static_cast<uint32_t>(-1);
    other.m_indicesCount = static_cast<uint32_t>(-1);
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    m_vulkanManager = other.m_vulkanManager;
    m_binding = other.m_binding;
    m_verticesCount = other.m_verticesCount;
    m_indicesCount = other.m_indicesCount;
    m_vertexBuffer = std::move(other.m_vertexBuffer);
    m_indexBuffer = std::move(other.m_indexBuffer);
    
    other.m_vulkanManager = nullptr;
    other.m_binding = static_cast<uint32_t>(-1);
    other.m_verticesCount = static_cast<uint32_t>(-1);
    other.m_indicesCount = static_cast<uint32_t>(-1);

    return *this;
}

uint32_t VertexArray::GetVerticesCount() const
{
    return m_verticesCount;
}

uint32_t VertexArray::GetIndicesCount() const
{
    return m_indicesCount;
}

const GPUBuffer& VertexArray::GetVertexBuffer() const
{
    MLC_ASSERT(m_vertexBuffer.IsUsable(), "Vertex Array not initialized.");

    return m_vertexBuffer;
}

const GPUBuffer& VertexArray::GetIndexBuffer() const
{
    MLC_ASSERT(m_indexBuffer.IsUsable(), "Vertex Array not initialized.");

    return m_indexBuffer;
}

VkVertexInputBindingDescription VertexArray::GetBindingDescription() const
{
    MLC_ASSERT(m_vertexBuffer.IsUsable(), "Vertex Array not initialized.");

    return VkVertexInputBindingDescription {
        .binding = m_binding,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX  // can be per instance
    };
}

std::array<VkVertexInputAttributeDescription, 3> VertexArray::GetAttribDescriptions() const
{
    MLC_ASSERT(m_vertexBuffer.IsUsable(), "Vertex Array not initialized.");

    VkVertexInputAttributeDescription positionAttribDesc {
        .location = 0,
        .binding = m_binding,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, position)
    };
    VkVertexInputAttributeDescription colorAttribDesc {
        .location = 1,
        .binding = m_binding,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, color)
    };
    VkVertexInputAttributeDescription uvAttribDesc {
        .location = 2,
        .binding = m_binding,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex, uv)
    };

    return std::array<VkVertexInputAttributeDescription, 3> {
        positionAttribDesc,
        colorAttribDesc,
        uvAttribDesc
    };
}

MLC_NAMESPACE_END