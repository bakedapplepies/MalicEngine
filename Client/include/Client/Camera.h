#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#undef near  // <- stupid remnant from windows
#undef far

namespace MalicClient
{

class Camera
{
public:
    Camera(const glm::vec3& position, const glm::vec3& direction, float near, float far, float pov);
    ~Camera() = default;

    [[nodiscard]] glm::mat4 GetViewMat() const;

    // Note: aspect[ratio] is an argument bc window dimensions my change its value
    // while other variables aren't changed.
    [[nodiscard]] glm::mat4 GetProjMat(float aspect) const;

public:
    glm::vec3 position;
    glm::vec3 direction;
    float near;
    float far;
    float pov;
};

}  // namespace MalicClient