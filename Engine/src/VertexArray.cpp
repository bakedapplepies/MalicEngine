#include "Engine/VertexArray.h"

#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

VertexArray::VertexArray(const VkDevice& device,
                         const VkPhysicalDevice& physical_device,
                         uint32_t binding,
                         const std::vector<uint32_t>& queue_families,
                         const std::vector<Vertex>& vertices)
    : m_device(device), m_binding(binding)
{
    VkBufferCreateInfo vertexBufferCreateInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = sizeof(Vertex) * vertices.size(),  // buffer size is explicitly-defined
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = static_cast<uint32_t>(queue_families.size()),
        .pQueueFamilyIndices = queue_families.data()
    };

    VkResult result = vkCreateBuffer(device,
                                     &vertexBufferCreateInfo,
                                     MLC_VULKAN_ALLOCATOR,
                                     &m_vertexBuffer);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to create vertex buffer.");

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, m_vertexBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = _FindMemoryType(physical_device,
                                           memoryRequirements.memoryTypeBits,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                         | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    result = vkAllocateMemory(device, &allocInfo, MLC_VULKAN_ALLOCATOR, &m_bufferMemory);
    MLC_ASSERT(result == VK_SUCCESS, "Failed to allocate vertex buffer memory.");

    VkBindBufferMemoryInfo bindBufferInfo;
    vkBindBufferMemory(device, m_vertexBuffer, m_bufferMemory, 0);
    // TODO: vkBindBufferMemory2: Bind multiple buffers at once
    // vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo *pBindInfos)

    void* mappedRegion;
    vkMapMemory(device, m_bufferMemory, 0, vertexBufferCreateInfo.size, 0, &mappedRegion);
    memcpy(mappedRegion, vertices.data(), static_cast<size_t>(vertexBufferCreateInfo.size));
    vkUnmapMemory(device, m_bufferMemory);
}

const VkBuffer& VertexArray::GetVertexBuffer() const
{
    return m_vertexBuffer;
}

void VertexArray::Deallocate() const
{
    vkDestroyBuffer(m_device, m_vertexBuffer, MLC_VULKAN_ALLOCATOR);
    vkFreeMemory(m_device, m_bufferMemory, MLC_VULKAN_ALLOCATOR);
}

VkVertexInputBindingDescription VertexArray::GetBindingDescription() const
{
    return VkVertexInputBindingDescription {
        .binding = m_binding,  // m_binding
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
}

std::array<VkVertexInputAttributeDescription, 2> VertexArray::GetAttribDescriptions() const
{
    VkVertexInputBindingDescription bindingDesc = GetBindingDescription();

    VkVertexInputAttributeDescription positionAttribDesc {
        .location = 0,
        .binding = bindingDesc.binding,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, position)
    };
    VkVertexInputAttributeDescription colorAttribDesc {
        .location = 1,
        .binding = bindingDesc.binding,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, color)
    };

    return std::array<VkVertexInputAttributeDescription, 2> {
        positionAttribDesc,
        colorAttribDesc
    };
}

uint32_t VertexArray::_FindMemoryType(const VkPhysicalDevice& physical_device,
                                      uint32_t type_filter,
                                      VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    MLC_ASSERT(false, "Failed to find suitable device memory type.");
    return static_cast<uint32_t>(-1);
}

MLC_NAMESPACE_END