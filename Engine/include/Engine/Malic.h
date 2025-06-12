#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <fmt/format.h>

#include "core/Defines.h"
#include "VulkanManager.h"

MLC_NAMESPACE_START

struct GLFWSharedResource
{
    VulkanManager* vulkanManager = nullptr;
};

class MalicEngine
{
public:
    void Run();

    MalicEngine() = default;
    ~MalicEngine() = default;

private:
    GLFWwindow* m_window = nullptr;
    VulkanManager m_vulkanManager;
    GLFWSharedResource m_glfwSharedResource;

private:
    void _WindowInit();
    void _MainLoop();
    void _ShutDown();
    void _DrawFrame(const std::vector<VertexArray>& vertex_arrays);
};

MLC_NAMESPACE_END
