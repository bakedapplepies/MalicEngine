#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/Defines.h"
#include "Engine/VulkanManager.h"
#include "Engine/ResourceManager.h"
#include "Engine/VertexArray.h"
#include "Engine/DescriptorInfo.h"
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

    MLC_NODISCARD const ResourceManager* GetResourceManager() const;

    MLC_NODISCARD bool IsKeyPressed(uint32_t key) const;
    MLC_NODISCARD glm::vec2 GetCursorPos() const;
    void HideCursor() const;

    MLC_NODISCARD const WindowInfo* GetWindowInfo() const;
    void SetUserPointer(void* data);
    MLC_NODISCARD void* GetUserPointer() const;

    MLC_NODISCARD VertexArray CreateVertexArray(const std::vector<Vertex>& vertices,
                                                const std::vector<uint16_t>& indices) const;
    void CreateDescriptors(const std::vector<DescriptorInfo>& descriptor_infos);
    MLC_NODISCARD UniformBuffer CreateUBO(uint32_t binding, VkDeviceSize size) const;
    void AssignPipeline(const PipelineResources& pipeline_config);
    void AssignRenderList(const std::vector<RenderResources>& render_list);

private:
    const WindowInfo m_windowInfo;
    GLFWwindow* m_window = nullptr;
    VulkanManager m_vulkanManager;
    ResourceManager m_resourceManager;
    GLFWSharedResource m_glfwSharedResource;
    std::vector<RenderResources> m_renderList;
    void* m_userData;

private:
    void _WindowInit();
    void _MainLoop();
    void _ShutDown();
    void _DrawFrame();
};

MLC_NAMESPACE_END
