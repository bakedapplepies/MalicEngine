#include "Engine/Material.h"

#include <utility>

MLC_NAMESPACE_START

Material::Material(const Shader& shader)
    : m_shader(shader)
{}

Material::Material(Material&& other) noexcept
{
    m_shader = std::move(other.m_shader);
    m_albedo = other.m_albedo;

    m_albedo = nullptr;
}

Material& Material::operator=(Material&& other) noexcept
{
    m_shader = std::move(other.m_shader);
    m_albedo = other.m_albedo;

    m_albedo = nullptr;

    return *this;
}

void Material::SetShader(const Shader& shader)
{
    m_shader = shader;
}

// TODO
void Material::SetAlbedo(const Texture2D* texture)
{
    m_albedo = texture;
}

const Shader* Material::GetShader() const
{
    return &m_shader;
}

const Texture2D* Material::GetAlbedo() const
{
    return m_albedo;
}

bool Material::IsUsable() const
{
    return m_shader.IsUsable() && m_albedo->IsUsable();
}

MLC_NAMESPACE_END