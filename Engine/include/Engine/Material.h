#pragma once

#include "Engine/core/Defines.h"
#include "Engine/Shader.h"
#include "Engine/Texture2D.h"

MLC_NAMESPACE_START

class Material
{
public:
    Material() = default;
    Material(const Shader& shader);
    ~Material() = default;
    Material(const Material&) = default;
    Material& operator=(const Material&) = default;
    Material(Material&& other) noexcept;
    Material& operator=(Material&& other) noexcept;

    void SetShader(const Shader& shader);
    void SetAlbedo(const Texture2D* texture);

    MLC_NODISCARD const Shader* GetShader() const;
    const Texture2D* GetAlbedo() const;

    MLC_NODISCARD bool IsUsable() const;

private:
    Shader m_shader;
    const Texture2D* m_albedo;
};
    
MLC_NAMESPACE_END