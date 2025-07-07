#pragma once

// #include <vector>

#include "Engine/core/Defines.h"
#include "Engine/Material.h"

MLC_NAMESPACE_START

class VertexArray;
struct PipelineResources
{
    Material material;
    const VertexArray* vertexArray = nullptr;
    // alpha blending config
};

MLC_NAMESPACE_END