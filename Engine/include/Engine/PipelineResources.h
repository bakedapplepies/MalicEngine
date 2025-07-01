#pragma once

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

class Shader;
class VertexArray;
struct PipelineResources
{
    const Shader* shader = nullptr;
    const VertexArray* vertexArrays = nullptr;
    // alpha blending config
};

MLC_NAMESPACE_END