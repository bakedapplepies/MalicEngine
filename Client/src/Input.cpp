#include "Client/Input.h"

#include "Client/Core.h"

namespace MalicClient
{

void ProcessInput(const Malic::MalicEngine *engine, float delta_time)
{
    MoveCamera(engine, delta_time);
    RotateCamera(engine, delta_time);
}

void MoveCamera(const Malic::MalicEngine* engine, float delta_time)
{
    static MyData* myData = static_cast<MyData*>(engine->GetUserPointer());

    if (engine->IsKeyPressed(GLFW_KEY_A))
    {
        myData->camera.position += glm::normalize(glm::cross(Malic::VEC3_UP, myData->camera.direction)) * delta_time;
    }
    if (engine->IsKeyPressed(GLFW_KEY_D))
    {
        myData->camera.position -= glm::normalize(glm::cross(Malic::VEC3_UP, myData->camera.direction)) * delta_time;
    }
    if (engine->IsKeyPressed(GLFW_KEY_W))
    {
        myData->camera.position += glm::normalize(glm::vec3(myData->camera.direction.x, 0.0f, myData->camera.direction.z)) * delta_time;
    }
    if (engine->IsKeyPressed(GLFW_KEY_S))
    {
        myData->camera.position -= glm::normalize(glm::vec3(myData->camera.direction.x, 0.0f, myData->camera.direction.z)) * delta_time;
    }
    if (engine->IsKeyPressed(GLFW_KEY_SPACE))
    {
        myData->camera.position += Malic::VEC3_UP * delta_time;
    }
    if (engine->IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
    {
        myData->camera.position -= Malic::VEC3_UP * delta_time;
    }
}

void RotateCamera(const Malic::MalicEngine* engine, float delta_time)
{
    static MyData* myData = static_cast<MyData*>(engine->GetUserPointer());

    glm::vec2 cursorPos = engine->GetCursorPos();

    static float pitch = glm::degrees(glm::asin(myData->camera.direction.y));
    static float yaw = glm::sign(myData->camera.direction.z) * glm::degrees(glm::acos(myData->camera.direction.x / glm::cos(glm::radians(pitch))));
    static float sensitivity = 0.065f;
    static float lastX = cursorPos.x;
    static float lastY = cursorPos.y;

    float deltaX = cursorPos.x - lastX;
    float deltaY = cursorPos.y - lastY;
    lastX = cursorPos.x;
    lastY = cursorPos.y;

    yaw += deltaX * sensitivity;
    pitch += deltaY * sensitivity;

    if (pitch > 89.5f) pitch = 89.5f;
    if (pitch < -89.5f) pitch = -89.5f;

    glm::vec3 direction;
    direction.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    direction.y = glm::sin(glm::radians(-pitch));
    direction.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    myData->camera.direction = direction;
    // std::cout << direction.x << ' ' << direction.y << ' ' << direction.z << '\n';
    // std::cout << cursorPos.y << ' ' << pitch << ' ' << direction.y << '\n';
}

}  // namespace MalicClient