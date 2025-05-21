#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <vulkan/vulkan.h>

#include "Engine/core/Defines.h"

MLC_NAMESPACE_START

class Shader
{
public:
    Shader(const std::string& vert_path, const std::string& frag_path, const VkDevice& device);
    ~Shader();

    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

private:
    VkDevice m_device;  // TODO: Set up ordered-initializations for scoped

private:
    MLC_NODISCARD std::vector<char> _ReadFile(std::ifstream& file);
    MLC_NODISCARD VkShaderModule _CreateShaderModule(const std::vector<char>& bytecode,
                                                     const VkDevice& device);
};

MLC_NAMESPACE_END