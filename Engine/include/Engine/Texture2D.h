#pragma once

#include "Engine/core/Defines.h"
#include "Engine/core/Filesystem.h"
#include "Engine/GPUImage.h"
#include "Engine/Image2DViewer.h"

MLC_NAMESPACE_START

class VulkanManager;
class Texture2D
{
public:
    Texture2D() = default;
    Texture2D(const VulkanManager* vulkan_manager, const File& file);
    ~Texture2D();
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(Texture2D&& other) noexcept;

    MLC_NODISCARD bool IsUsable() const;

private:
    const VulkanManager* m_vulkanManager = nullptr;
    GPUImage m_image;
    Image2DViewer m_viewer;
};

MLC_NAMESPACE_END