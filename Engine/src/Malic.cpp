#include "Engine/Malic.h"

#include <Engine/core/Assert.h>
#include <Engine/core/Debug.h>

MLC_NAMESPACE_START

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

void MalicEngine::Run()
{
    _WindowInit();
    m_vulkanManager.Init(m_window);
    m_vulkanManager.ShutDown();
    // _MainLoop();
    _ShutDown();
}

void MalicEngine::_WindowInit()
{
    int glfwStatus = glfwInit();
    MLC_ASSERT(glfwStatus == GLFW_TRUE, "Failed to initialize GLFW.");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Malic Engine", nullptr, nullptr);
    glfwSetErrorCallback(glfwErrorCallback);
}

void MalicEngine::_MainLoop()
{
    while(!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void MalicEngine::_ShutDown()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

MLC_NAMESPACE_END