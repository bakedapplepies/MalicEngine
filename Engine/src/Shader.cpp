#include "Engine/Shader.h"

MLC_NAMESPACE_START

Shader::Shader(VkShaderModule vert_module, VkShaderModule frag_module)
    : m_vertShaderModule(vert_module), m_fragShaderModule(frag_module)
{}

Shader::~Shader()
{}

bool Shader::IsUsable() const
{
    return m_vertShaderModule != VK_NULL_HANDLE &&
           m_fragShaderModule != VK_NULL_HANDLE;
}

uint32_t Shader::GetActiveStages() const
{
    uint32_t activeStages = 2;
    if (m_geomShaderModule != VK_NULL_HANDLE) activeStages++;

    return activeStages;
}

MLC_NAMESPACE_END