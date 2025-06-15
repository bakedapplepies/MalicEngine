#include "Client/Camera.h"

#include "Engine/core/Config.h"

Camera::Camera(const glm::vec3& position, const glm::vec3& direction, float near, float far, float pov)
    : position(position), direction(direction), near(near), far(far), pov(pov)
{}

glm::mat4 Camera::GetViewMat() const
{
    return glm::lookAt(position, position + direction, Malic::VEC3_UP);
}

glm::mat4 Camera::GetProjMat(float aspect) const
{
    
    return glm::perspective(glm::radians(pov), aspect, near, far);
}