#include "Engine/GPUImage.h"

#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

GPUImage::~GPUImage()
{
    MLC_ASSERT(m_handle == VK_NULL_HANDLE,
               fmt::format("GPUImage handle [{}] was not destroyed.", static_cast<void*>(m_handle)));
    MLC_ASSERT(m_memory == VK_NULL_HANDLE,
               fmt::format("GPUImage memory [{}] was not deallocated.", static_cast<void*>(m_memory)));
}

GPUImage::GPUImage(GPUImage&& other) noexcept
{
    m_handle = other.m_handle;
    m_memory = other.m_memory;
    m_properties = other.m_properties;

    other.m_handle = VK_NULL_HANDLE;
    other.m_memory = VK_NULL_HANDLE;
    other.m_properties = 0;
}

GPUImage& GPUImage::operator=(GPUImage&& other) noexcept
{
    m_handle = other.m_handle;
    m_memory = other.m_memory;
    m_properties = other.m_properties;

    other.m_handle = VK_NULL_HANDLE;
    other.m_memory = VK_NULL_HANDLE;
    other.m_properties = 0;

    return *this;
}

bool GPUImage::IsUsable() const
{
    return m_handle != VK_NULL_HANDLE && m_memory != VK_NULL_HANDLE;
}

MLC_NAMESPACE_END