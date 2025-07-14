#include "Engine/Texture2D.h"

#include <stb/stb_image.h>

#include "Engine/core/Assert.h"
#include "Engine/VulkanManager.h"

MLC_NAMESPACE_START

// https://stackoverflow.com/questions/50403342/how-do-i-properly-use-stdstring-on-utf-8-in-c

Texture2D::Texture2D(const VulkanManager* vulkan_manager, const File& file)
    : m_vulkanManager(vulkan_manager)
{
    int width, height, channels;
    
#ifndef _WIN32
    FILE* f = fopen(file.c_str(), "rb");
#else
    std::string s(file.GetPath());
    std::wstring filePathW;
    filePathW.resize(s.length());
    int newSize = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.size(), const_cast<wchar_t *>(filePathW.c_str()), filePathW.length());
    filePathW.resize(newSize);
    FILE* f = _wfopen(filePathW.c_str(), L"rb");
#endif

    stbi_uc* pixels = stbi_load_from_file(f, &width, &height, &channels, STBI_rgb_alpha);
    MLC_ASSERT(pixels != nullptr, fmt::format("Failed to load texture image data.\n{}", stbi_failure_reason()));
    fclose(f);

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
    Bind();
}

Texture2D::~Texture2D()
{
    if (m_vulkanManager)
    {
        m_vulkanManager->DestroyImage2DViewer(m_viewer);
        m_vulkanManager->DeallocateImage2D(m_image);
    }
}

Texture2D::Texture2D(Texture2D&& other) noexcept
{
    m_vulkanManager = other.m_vulkanManager;
    m_image = std::move(other.m_image);
    m_viewer = std::move(other.m_viewer);

    other.m_vulkanManager = nullptr;
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
{
    m_vulkanManager = other.m_vulkanManager;
    m_image = std::move(other.m_image);
    m_viewer = std::move(other.m_viewer);

    other.m_vulkanManager = nullptr;

    return *this;
}

bool Texture2D::IsUsable() const
{
    return m_image.IsUsable() && m_viewer.IsUsable();
}

void Texture2D::Bind() const
{
    m_vulkanManager->DescriptorSetBindImage2D(m_viewer);
}

MLC_NAMESPACE_END