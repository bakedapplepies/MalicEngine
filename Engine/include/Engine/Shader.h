#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

class Shader
{
friend class VulkanManager;
public:
    Shader() = default;
    Shader(VkShaderModule vert_module, VkShaderModule frag_module);
    ~Shader();
    Shader(const Shader&) = default;
    Shader& operator=(const Shader&) = default;
    Shader(Shader&& other) noexcept = default;
    Shader& operator=(Shader&& other) noexcept = default;

    MLC_NODISCARD bool IsUsable() const;
    MLC_NODISCARD uint32_t GetActiveStages() const;

private:
    VkShaderModule m_vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule m_fragShaderModule = VK_NULL_HANDLE;
    VkShaderModule m_geomShaderModule = VK_NULL_HANDLE;
};

MLC_NAMESPACE_END