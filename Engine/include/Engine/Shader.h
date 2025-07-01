#pragma once

#include <string>
#include <fstream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Engine/core/Defines.h"
#include "Engine/VulkanManager.h"

MLC_NAMESPACE_START

class Shader
{
friend class VulkanManager;
public:
    Shader(const VulkanManager* vulkan_manager, const std::string& vert_path, const std::string& frag_path);
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

private:
    const VulkanManager* m_vulkanManager = nullptr;
    VkShaderModule m_vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule m_fragShaderModule = VK_NULL_HANDLE;

private:
    MLC_NODISCARD std::vector<char> _ReadFile(std::ifstream& file);
    MLC_NODISCARD VkShaderModule _CreateShaderModule(const std::vector<char>& bytecode,
                                                     const VkDevice& device);
};

MLC_NAMESPACE_END