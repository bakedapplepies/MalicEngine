#include "Engine/ResourceManager.h"

#include <unordered_map>
#include <fstream>

#include "Engine/VulkanManager.h"
#include "Engine/core/Assert.h"
#include "Engine/core/Logging.h"

MLC_NAMESPACE_START

// The only benefit to making all these containers
// static is that there are private methods only VulkanManager
// can access.
static std::unordered_map<const char*, uint16_t> s_vertModuleIndices;
static std::unordered_map<const char*, uint16_t> s_fragModuleIndices;
static std::unordered_map<const char*, uint16_t> s_texture2DIndices;
static std::vector<VkShaderModule> s_vertModules;
static std::vector<VkShaderModule> s_fragModules;
static std::vector<Texture2D> s_texture2Ds;

void ResourceManager::_Init(const VulkanManager* vulkan_manager)
{
    if (m_vulkanManager)
    {
        MLC_ERROR("Already initialized ResourceManager.");
        return;
    }
    m_vulkanManager = vulkan_manager;
}

void ResourceManager::_ShutDown()
{
    s_vertModuleIndices.clear();
    s_fragModuleIndices.clear();
    s_texture2DIndices.clear();
    for (uint32_t i = 0; i < s_vertModules.size(); i++)
    {
        m_vulkanManager->DestroyShaderModule(s_vertModules[i]);
    }
    for (uint32_t i = 0; i < s_fragModules.size(); i++)
    {
        m_vulkanManager->DestroyShaderModule(s_fragModules[i]);
    }
    s_vertModules.clear();
    s_fragModules.clear();
    s_texture2Ds.clear();
    m_vulkanManager = nullptr;
}

Shader ResourceManager::GetShader(const File& vert_file, const File& frag_file) const
{
    if (!s_vertModuleIndices[vert_file.GetPath()])
    {
        VkShaderModule vertModule;
        s_vertModuleIndices[vert_file.GetPath()] = s_vertModuleIndices.size() - 1;
        m_vulkanManager->CreateShaderModule(vertModule, _GetFileBytecode(vert_file));
        s_vertModules.push_back(vertModule);
    }
    if (!s_fragModuleIndices[frag_file.GetPath()])
    {
        VkShaderModule fragModule;
        s_fragModuleIndices[frag_file.GetPath()] = s_fragModuleIndices.size() - 1;
        m_vulkanManager->CreateShaderModule(fragModule, _GetFileBytecode(frag_file));
        s_fragModules.push_back(fragModule);
    }

    return Shader(
        s_vertModules[s_vertModuleIndices[vert_file.GetPath()]],
        s_fragModules[s_fragModuleIndices[frag_file.GetPath()]]
    );
}

const Texture2D* ResourceManager::GetTexture2D(const File& file) const
{
    if (!s_texture2DIndices[file.GetPath()])
    {
        s_texture2Ds.emplace_back(m_vulkanManager, file.GetPath());
        s_texture2DIndices[file.GetPath()] = s_texture2Ds.size() - 1;
    }
    return &s_texture2Ds[s_texture2DIndices[file.GetPath()]];
}

std::vector<char> ResourceManager::_GetFileBytecode(const File& file) const
{
    std::ifstream fileStream(file.GetPath(), std::ios::binary | std::ios::ate);
    MLC_ASSERT(fileStream.is_open(), fmt::format("Failed to open vertex shader \"{}\".", file.GetPath()));

    size_t fileSize = static_cast<size_t>(fileStream.tellg());
    MLC_ASSERT(fileSize != std::numeric_limits<size_t>::max(), "");
    std::vector<char> buffer(fileSize);

    fileStream.seekg(0);
    fileStream.read(buffer.data(), fileSize);

    fileStream.close();

    return buffer;
}

MLC_NAMESPACE_END