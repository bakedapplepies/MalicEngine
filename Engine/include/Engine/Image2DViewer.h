#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

class Image2DViewer
{
friend class VulkanManager;
public:
    Image2DViewer() = default;
    ~Image2DViewer();
    Image2DViewer(const Image2DViewer&) = delete;
    Image2DViewer& operator=(const Image2DViewer&) = delete;
    Image2DViewer(Image2DViewer&& other) noexcept;
    Image2DViewer& operator=(Image2DViewer&& other) noexcept;

    bool IsUsable() const;

private:
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
};

MLC_NAMESPACE_END