#include "Engine/VertexArray.h"

#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

VertexArray::VertexArray(const VulkanManager* vulkan_manager,
                         uint32_t binding,
                         const std::vector<Vertex>& vertices)
    : m_vulkanManager(vulkan_manager), m_binding(binding), m_verticesCount(static_cast<uint32_t>(vertices.size()))
{
    // TODO: vkBindBufferMemory2: Bind multiple buffers at once
    // vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos)

    GPUBuffer stagingBuffer;
    m_vulkanManager->AllocateBuffer(stagingBuffer,
                                    vertices.size() * sizeof(Vertex),
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);    
    m_vulkanManager->AllocateBuffer(m_buffer,
                                    vertices.size() * sizeof(Vertex),
                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_vulkanManager->UploadBuffer(stagingBuffer,
                                  vertices.data(),
                                  sizeof(Vertex) * vertices.size());
    m_vulkanManager->CopyBuffer(stagingBuffer, m_buffer, sizeof(Vertex) * vertices.size());
    m_vulkanManager->DeallocateBuffer(stagingBuffer);
}

VertexArray::~VertexArray()
{
    // Note: When performing move semantics, buffer handles with value VK_NULL_HANDLE
    // can still be safely called with vkDestroyBuffer, but of course it can't be
    // used with other functions, which can prevent implicitly ownership of buffers
    m_vulkanManager->DeallocateBuffer(m_buffer);
}

VertexArray::VertexArray(VertexArray&& other) noexcept
{
    other.m_vulkanManager = m_vulkanManager;
    other.m_binding = m_binding;
    other.m_verticesCount = m_verticesCount;
    other.m_buffer = std::move(m_buffer);

    m_vulkanManager = nullptr;
    m_binding = static_cast<uint32_t>(-1);
    m_verticesCount = static_cast<uint32_t>(-1);
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    other.m_vulkanManager = m_vulkanManager;
    other.m_binding = m_binding;
    other.m_verticesCount = m_verticesCount;
    other.m_buffer = std::move(m_buffer);
    
    m_vulkanManager = nullptr;
    m_binding = static_cast<uint32_t>(-1);
    m_verticesCount = static_cast<uint32_t>(-1);

    return *this;
}

uint32_t VertexArray::GetVerticesCount() const
{
    return m_verticesCount;
}

const GPUBuffer& VertexArray::GetGPUBuffer() const
{
    MLC_ASSERT(m_buffer.IsUsable(), "Vertex Array not initialized.");

    return m_buffer;
}

VkVertexInputBindingDescription VertexArray::GetBindingDescription() const
{
    MLC_ASSERT(m_buffer.IsUsable(), "Vertex Array not initialized.");

    return VkVertexInputBindingDescription {
        .binding = m_binding,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX  // can be per instance
    };
}

std::array<VkVertexInputAttributeDescription, 2> VertexArray::GetAttribDescriptions() const
{
    MLC_ASSERT(m_buffer.IsUsable(), "Vertex Array not initialized.");

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

    return std::array<VkVertexInputAttributeDescription, 2> {
        positionAttribDesc,
        colorAttribDesc
    };
}

MLC_NAMESPACE_END