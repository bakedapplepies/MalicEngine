#pragma once

#include <vector>

#include "Engine/core/Defines.h"
#include "Engine/Material.h"

MLC_NAMESPACE_START

class VertexArray;
struct RenderResources
{
    Material material;
    const VertexArray* vertexArray = nullptr;
    std::vector<uint32_t> indexOffset;
    std::vector<uint32_t> indexCount;

};

MLC_NAMESPACE_END