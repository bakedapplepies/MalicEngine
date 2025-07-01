#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/Defines.h"
#include "Engine/VulkanManager.h"
#include "Engine/VertexArray.h"
#include "Engine/Shader.h"
#include "Engine/DescriptorInfo.h"
#include "Engine/Texture2D.h"
#include "Engine/UniformBuffer.h"

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
    MLC_NODISCARD glm::vec2 GetCursorPos() const;
    void HideCursor() const;

    MLC_NODISCARD const WindowInfo* GetWindowInfo() const;
    void SetUserPointer(void* data);
    MLC_NODISCARD void* GetUserPointer() const;

    MLC_NODISCARD VertexArray CreateVertexArray(uint32_t binding,
                                                const std::vector<Vertex>& vertices,
                                                const std::vector<uint16_t>& indices) const;
    MLC_NODISCARD Shader CreateShader(const std::string& vert_path, const std::string& frag_path) const;
    void CreateDescriptors(const std::vector<DescriptorInfo>& descriptor_infos);
    MLC_NODISCARD Texture2D CreateTexture2D(const char* path) const;
    MLC_NODISCARD UniformBuffer CreateUBO(uint32_t binding, VkDeviceSize size) const;
    void AssignPipeline(const Malic::PipelineResources& pipeline_config);

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
