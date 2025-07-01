#pragma once

#include <vector>

#include "Client/Camera.h"
#include "Engine/VertexArray.h"
#include "Engine/UniformBuffer.h"
#include "Engine/Texture2D.h"

namespace MalicClient
{

class ApplicationData
{
public:
    ApplicationData() = default;
    ~ApplicationData() = default;
    ApplicationData(const ApplicationData&) = delete;
    ApplicationData& operator=(const ApplicationData&) = delete;
    ApplicationData(ApplicationData&& other) noexcept;
    ApplicationData& operator=(ApplicationData&& other) noexcept;

public:
    std::vector<Malic::VertexArray> vertexArrays;
    Malic::UniformBuffer uniformBuffer;
    Malic::Texture2D texture;
    
    MalicClient::Camera camera;
};

}