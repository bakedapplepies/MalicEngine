#include "Engine/GPUBuffer.h"

#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

GPUBuffer::~GPUBuffer()
{
    MLC_ASSERT(m_handle == VK_NULL_HANDLE,
               fmt::format("GPUBuffer handle [{}] was not destroyed.", static_cast<void*>(m_handle)));
    MLC_ASSERT(m_memory == VK_NULL_HANDLE,
               fmt::format("GPUBuffer memory [{}] was not deallocated.", static_cast<void*>(m_memory)));
}

GPUBuffer::GPUBuffer(GPUBuffer&& other) noexcept
{
    other.m_handle = m_handle;
    other.m_memory = m_memory;

    m_handle = VK_NULL_HANDLE;
    m_memory = VK_NULL_HANDLE;
}

GPUBuffer& GPUBuffer::operator=(GPUBuffer&& other) noexcept
{
    other.m_handle = m_handle;
    other.m_memory = m_memory;

    m_handle = VK_NULL_HANDLE;
    m_memory = VK_NULL_HANDLE;

    return *this;
}

bool GPUBuffer::IsUsable() const
{
    return m_handle != VK_NULL_HANDLE && m_memory != VK_NULL_HANDLE;
}

MLC_NAMESPACE_END