#include "Engine/Texture2D.h"

#include "Engine/core/Assert.h"

#include <stb/stb_image.h>

MLC_NAMESPACE_START

Texture2D::Texture2D(const VulkanManager* vulkan_manager, const char* path)
    : m_vulkanManager(vulkan_manager)
{
    int width, height, channels;
    stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

    MLC_ASSERT(pixels != nullptr, "Failed to load texture image data.");

    VkDeviceSize size = width * height * 4;

    GPUBuffer stagingBuffer;
    vulkan_manager->AllocateBuffer(stagingBuffer,
                                   size,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vulkan_manager->UploadBuffer(stagingBuffer, pixels, size);
    stbi_image_free(pixels);
    vulkan_manager->AllocateImage2D(m_image,
                                    width,
                                    height,
                                    VK_FORMAT_R8G8B8A8_SRGB,
                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vulkan_manager->TransitionImageLayout(m_image,
                                          VK_FORMAT_R8G8B8A8_SRGB,
                                          VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vulkan_manager->CopyBufferToImage(stagingBuffer, m_image, width, height);
    vulkan_manager->TransitionImageLayout(m_image,
                                          VK_FORMAT_B8G8R8A8_SRGB,
                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vulkan_manager->DeallocateBuffer(stagingBuffer);

    vulkan_manager->CreateImage2DViewer(m_viewer, m_image, VK_FORMAT_R8G8B8A8_SRGB);
    vulkan_manager->DescriptorSetBindImage2D(m_viewer);
}

Texture2D::~Texture2D()
{
    m_vulkanManager->DestroyImage2DViewer(m_viewer);
    m_vulkanManager->DeallocateImage2D(m_image);
}

MLC_NAMESPACE_END