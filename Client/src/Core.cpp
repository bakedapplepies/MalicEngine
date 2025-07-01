#include "Client/Core.h"

namespace MalicClient
{
 
ApplicationData::ApplicationData(ApplicationData&& other) noexcept
{
    vertexArrays = std::move(other.vertexArrays);
    uniformBuffer = std::move(other.uniformBuffer);
    texture = std::move(other.texture);

    camera = other.camera;
    other.camera = Camera();
}

ApplicationData& ApplicationData::operator=(ApplicationData&& other) noexcept
{
    vertexArrays = std::move(other.vertexArrays);
    uniformBuffer = std::move(other.uniformBuffer);
    texture = std::move(other.texture);
    
    camera = other.camera;
    other.camera = Camera();
    
    return *this;
}

}