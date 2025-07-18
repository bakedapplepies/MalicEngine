#pragma once

#include <vector>

#include "Engine/core/Defines.h"
#include "Engine/Material.h"

MLC_NAMESPACE_START

struct PipelineResources
{
    Material material;
    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescs;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttribDescs;
    // alpha blending config
};

MLC_NAMESPACE_END