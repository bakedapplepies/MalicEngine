#pragma once

#include <vector>

#include "Engine/core/Defines.h"
#include "Engine/core/Filesystem.h"
#include "Engine/Shader.h"
#include "Engine/Texture2D.h"

MLC_NAMESPACE_START

class MalicEngine;
class ResourceManager
{
friend class MalicEngine;
public:
    Shader GetShader(const File& vert_file, const File& frag_file) const;
    const Texture2D* GetTexture2D(const File& file) const;
    
private:
    ResourceManager() = default;
    ~ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    void _Init(const VulkanManager* vulkan_manager);
    void _ShutDown();
    
    std::vector<char> _GetFileBytecode(const File& file) const;

private:
    const VulkanManager* m_vulkanManager = nullptr;
};

MLC_NAMESPACE_END