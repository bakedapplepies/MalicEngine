#include "Engine/core/Logging.h"

std::ostream& operator<<(std::ostream& stream, const glm::vec2& vec2)
{
    std::cout << glm::to_string(vec2);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const glm::vec3& vec3)
{
    std::cout << glm::to_string(vec3);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const glm::mat3& mat3)
{
    std::cout << glm::to_string(mat3);
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const glm::mat4& mat4)
{
    std::cout << glm::to_string(mat4);
    return stream;
}