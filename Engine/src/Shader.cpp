#include "Engine/Shader.h"

#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

Shader::Shader(const VulkanManager* vulkan_manager, const std::string& vert_path, const std::string& frag_path)
    : m_vulkanManager(vulkan_manager)
{
    std::ifstream vert_file(vert_path, std::ios::binary | std::ios::ate);
    std::ifstream frag_file(frag_path, std::ios::binary | std::ios::ate);
    MLC_ASSERT(vert_file.is_open(), fmt::format("Failed to open vertex shader \"{}\".", vert_path));
    MLC_ASSERT(frag_file.is_open(), fmt::format("Failed to open fragment shader \"{}\".", frag_path));

    std::vector<char> vertBytecode = _ReadFile(vert_file);
    std::vector<char> fragBytecode = _ReadFile(frag_file);
    vert_file.close();
    frag_file.close();

    m_vulkanManager->CreateShaderModule(m_vertShaderModule, vertBytecode);
    m_vulkanManager->CreateShaderModule(m_fragShaderModule, fragBytecode);
}

Shader::~Shader()
{
    m_vulkanManager->DestroyShaderModule(m_vertShaderModule);
    m_vulkanManager->DestroyShaderModule(m_fragShaderModule);
}

std::vector<char> Shader::_ReadFile(std::ifstream& file)
{
    size_t fileSize = static_cast<size_t>(file.tellg());
    MLC_ASSERT(fileSize != std::numeric_limits<size_t>::max(), "");
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    return buffer;
}

MLC_NAMESPACE_END