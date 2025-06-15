#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/Defines.h"
#include "Engine/VulkanManager.h"

MLC_NAMESPACE_START

struct GLFWSharedResource
{
    VulkanManager* vulkanManager = nullptr;
};

class MalicEngine
{
public:
    struct WindowInfo
    {
        int width;
        int height;
        const char* title;
    };

public:
    MalicEngine(const WindowInfo& window_info);
    ~MalicEngine() = default;

    void Run();
    void ShutDown();

    MLC_NODISCARD bool IsKeyPressed(uint32_t key) const;

    MLC_NODISCARD const WindowInfo* GetWindowInfo() const;
    void SetUserPointer(void* data);
    MLC_NODISCARD void* GetUserPointer() const;
    MLC_NODISCARD VulkanManager* GetMutVulkanManager();

private:
    const WindowInfo m_windowInfo;
    GLFWwindow* m_window = nullptr;
    VulkanManager m_vulkanManager;
    GLFWSharedResource m_glfwSharedResource;
    void* m_userData;

private:
    void _WindowInit();
    void _MainLoop();
    void _ShutDown();
    void _DrawFrame();
};

MLC_NAMESPACE_END
