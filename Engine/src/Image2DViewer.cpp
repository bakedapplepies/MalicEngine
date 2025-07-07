#include "Engine/Image2DViewer.h"

#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

Image2DViewer::~Image2DViewer()
{
    MLC_ASSERT(m_imageView == VK_NULL_HANDLE,
               fmt::format("Image2DViewer image view [{}] was not destroyed.", static_cast<void*>(m_imageView)));
    MLC_ASSERT(m_sampler == VK_NULL_HANDLE,
               fmt::format("Image2DViewer sampler [{}] was not destroyed.", static_cast<void*>(m_sampler)));
}

Image2DViewer::Image2DViewer(Image2DViewer&& other) noexcept
{
    m_imageView = other.m_imageView;
    m_sampler = other.m_sampler;

    other.m_imageView = VK_NULL_HANDLE;
    other.m_sampler = VK_NULL_HANDLE;
}

Image2DViewer& Image2DViewer::operator=(Image2DViewer&& other) noexcept
{
    m_imageView = other.m_imageView;
    m_sampler = other.m_sampler;

    other.m_imageView = VK_NULL_HANDLE;
    other.m_sampler = VK_NULL_HANDLE;

    return *this;
}

bool Image2DViewer::IsUsable() const
{
    return m_imageView != VK_NULL_HANDLE &&
           m_sampler != VK_NULL_HANDLE;
}

MLC_NAMESPACE_END