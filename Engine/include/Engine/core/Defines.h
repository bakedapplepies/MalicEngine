#pragma once

struct VkAllocationCallbacks;

#define MLC_NAMESPACE_START namespace Malic {
#define MLC_NAMESPACE_END }
#if __cplusplus >= 201703L
    #define MLC_NODISCARD [[nodiscard]]
#else
    #define MLC_NODISCARD
#endif
const extern VkAllocationCallbacks* MLC_VULKAN_ALLOCATOR;