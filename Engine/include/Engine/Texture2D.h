#pragma once

#include "Engine/core/Defines.h"
#include "Engine/VulkanManager.h"
#include "Engine/GPUImage.h"
#include "Engine/Image2DViewer.h"

MLC_NAMESPACE_START

class Texture2D
{
public:
    Texture2D(const VulkanManager* vulkan_manager, const char* path);
    ~Texture2D();

private:
    const VulkanManager* m_vulkanManager = nullptr;
    GPUImage m_image;
    Image2DViewer m_viewer;
};

MLC_NAMESPACE_END