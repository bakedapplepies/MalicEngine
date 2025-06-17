#pragma once

#include <vector>
#include <memory>
#include "Client/Camera.h"
#include "Engine/VertexArray.h"
#include "Engine/UniformBuffer.h"

namespace MalicClient
{

struct MyData
{
    std::vector<Malic::VertexArray> vertexArrays;
    std::unique_ptr<Malic::UniformBuffer> uniformBuffer;
    
    MalicClient::Camera camera;
};

}