#include "Engine/Malic.h"

#include "Engine/core/Assert.h"
#include "Engine/core/Debug.h"
#include "Engine/core/Logging.h"
#include "Engine/MalicEntry.h"

#ifndef MLC_ASAN_DETECT_LEAKS
#   ifdef __cplusplus
extern "C"
#   endif
const char* __asan_default_options() { return "detect_leaks=0"; }
#endif

MLC_NAMESPACE_START

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_M:
                // glfwWindowHint(int hint, int value)
            default:
                break;
        }
    }
}

static void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
    GLFWSharedResource* glfwSharedResource = static_cast<GLFWSharedResource*>(glfwGetWindowUserPointer(window));
    glfwSharedResource->vulkanManager->ResizeFramebuffer();
}

MalicEngine::MalicEngine(const WindowInfo& window_info)
    : m_windowInfo(window_info)
{}

const MalicEngine::WindowInfo* MalicEngine::GetWindowInfo() const
{
    return &m_windowInfo;
}

void MalicEngine::SetUserPointer(void* data)
{
    m_userData = data;
}

void* MalicEngine::GetUserPointer() const
{
    return m_userData;
}

VulkanManager* MalicEngine::GetMutVulkanManager()
{
    return &m_vulkanManager;
}

void MalicEngine::Run()
{
    if (m_window)
    {
        MLC_ERROR("Malic Engine is already running.");
        return;
    }

    _WindowInit();
    m_vulkanManager.Init(m_window);
    MalicEntry(this);
    _MainLoop();  // Everything has to live & die inside this Loop to ensure proper resource management
}

void MalicEngine::ShutDown()
{
    m_vulkanManager.ShutDown();
    _ShutDown();
}

bool MalicEngine::IsKeyPressed(uint32_t key) const
{
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

glm::vec2 MalicEngine::GetCursorPos() const
{
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    return { x, y };
}

void MalicEngine::HideCursor() const
{
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void MalicEngine::_WindowInit()
{
    int glfwStatus = glfwInit();
    MLC_ASSERT(glfwStatus == GLFW_TRUE, "Failed to initialize GLFW.");

    m_glfwSharedResource.vulkanManager = &m_vulkanManager;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    m_window = glfwCreateWindow(m_windowInfo.width, m_windowInfo.height, m_windowInfo.title, nullptr, nullptr);
    glfwSetErrorCallback(glfwErrorCallback);
    glfwSetKeyCallback(m_window, key_callback);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_resize_callback);
    glfwSetWindowUserPointer(m_window, &m_glfwSharedResource);
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

        MalicUpdate(this, deltaTime);
        glfwPollEvents();
        _DrawFrame();
    }

    // All non-vulkan-initialization objects live here,
    // so wait for them to finish being used
    m_vulkanManager.WaitIdle();  // <-- always gotta be at the bottom here
}

void MalicEngine::_ShutDown()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();

    m_window = nullptr;
}

void MalicEngine::_DrawFrame()
{
    m_vulkanManager.Present();
}

MLC_NAMESPACE_END