#pragma once

#include <vector>
#include <memory>
#include "Client/Camera.h"
#include "Engine/VertexArray.h"
#include "Engine/UniformBuffer.h"
#include "Engine/Texture2D.h"

namespace MalicClient
{

struct ApplicationData
{
    std::vector<Malic::VertexArray> vertexArrays;
    std::unique_ptr<Malic::UniformBuffer> uniformBuffer;
    std::unique_ptr<Malic::Texture2D> texture;
    
    MalicClient::Camera camera;
};

}