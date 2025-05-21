#include "Engine/Shader.h"
#include "Engine/core/Assert.h"

MLC_NAMESPACE_START

Shader::Shader(const std::string& vert_path, const std::string& frag_path, const VkDevice& device)
    : m_device(device)
{
    std::ifstream vert_file(vert_path, std::ios::binary | std::ios::ate);
    std::ifstream frag_file(frag_path, std::ios::binary | std::ios::ate);
    MLC_ASSERT(vert_file.is_open(), fmt::format("Failed to open vertex shader \"{}\".", vert_path));
    MLC_ASSERT(frag_file.is_open(), fmt::format("Failed to open fragment shader \"{}\".", frag_path));

    std::vector<char> vertByteCode = _ReadFile(vert_file);
    std::vector<char> fragByteCode = _ReadFile(frag_file);
    vert_file.close();
    frag_file.close();

    vertShaderModule = _CreateShaderModule(vertByteCode, m_device);
    fragShaderModule = _CreateShaderModule(fragByteCode, m_device);
}

Shader::~Shader()
{
    vkDestroyShaderModule(m_device, vertShaderModule, MLC_VULKAN_ALLOCATOR);
    vkDestroyShaderModule(m_device, fragShaderModule, MLC_VULKAN_ALLOCATOR);
}

std::vector<char> Shader::_ReadFile(std::ifstream& file)
{
    size_t fileSize = static_cast<size_t>(file.tellg());
    MLC_ASSERT(fileSize != std::numeric_limits<size_t>::max(), "");
    fmt::print("fileSize: {}\n", fileSize);
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    return buffer;
}

VkShaderModule Shader::_CreateShaderModule(const std::vector<char>& bytecode,
                                           const VkDevice& device)
{
    VkShaderModule shaderModule;
    VkShaderModuleCreateInfo shaderModuleCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = bytecode.size(),
        .pCode = reinterpret_cast<const uint32_t*>(bytecode.data())
    };

    VkResult result = vkCreateShaderModule(device,
                                           &shaderModuleCreateInfo,
                                           MLC_VULKAN_ALLOCATOR,
                                           &shaderModule);
    return shaderModule;
}

MLC_NAMESPACE_END