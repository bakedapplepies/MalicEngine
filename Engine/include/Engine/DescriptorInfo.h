#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Engine/core/Defines.h"
#include "Engine/ShaderStages.h"

MLC_NAMESPACE_START

enum class DescriptorTypes
{
    UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    COMBINED_IMAGE_SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
};

struct DescriptorInfo
{
    DescriptorTypes type;
    ShaderStages stageFlags;
    uint32_t binding;
    uint32_t count;
};

MLC_NAMESPACE_END