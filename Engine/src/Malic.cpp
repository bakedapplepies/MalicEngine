#include "Engine/Malic.h"

#include <Engine/core/Assert.h>
#include <Engine/core/Debug.h>

MLC_NAMESPACE_START

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

#ifndef MLC_ASAN_DETECT_LEAKS
#   ifdef __cplusplus
extern "C"
#   endif
const char* __asan_default_options() { return "detect_leaks=0"; }
#endif

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void MalicEngine::Run()
{
    _WindowInit();
    m_vulkanManager.Init(m_window);
    _MainLoop();
    m_vulkanManager.ShutDown();
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
    glfwSetKeyCallback(m_window, key_callback);
}

void MalicEngine::_MainLoop()
{
    static uint32_t fps = 0;
    static float timeBegin = glfwGetTime();
    static float timeTotal = 0.0f;
    float deltaTime;
    while(!glfwWindowShouldClose(m_window))
    {
        deltaTime = glfwGetTime() - timeBegin;
        timeBegin = glfwGetTime();
        timeTotal += deltaTime;
        fps++;
        if (timeTotal >= 1.0f)
        {
            fmt::print("FPS: {}\n", fps);
            fps = 0;
            timeTotal = 0.0f;
        }

        glfwPollEvents();
        _DrawFrame();
    }
    m_vulkanManager.WaitIdle();
}

void MalicEngine::_ShutDown()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void MalicEngine::_DrawFrame()
{
    m_vulkanManager.WaitAndResetFence();
    m_vulkanManager.Present();
}

MLC_NAMESPACE_END