#pragma once

#include <vector>

#include <assimp/scene.h>

#include "Engine/Malic.h"
#include "Engine/VertexArray.h"
#include "Engine/Material.h"

namespace MalicClient
{

class Model
{
public:
    Model(const Malic::MalicEngine* engine, const char* path);
    ~Model();

public:
    [[nodiscard]] Malic::RenderResources GetRenderResources() const;

private:
    void _GetMaterials(const aiScene* scene, const Malic::MalicEngine* engine);
    void _ProcessNode(const aiNode* node,
                      const aiScene* scene,
                      std::vector<Malic::Vertex>& vertices,
                      std::vector<uint16_t>& indices) const;
    void _ProcessMesh(const aiMesh* mesh,
                      std::vector<Malic::Vertex>& vertices,
                      std::vector<uint16_t>& indices) const;

private:
    // TODO: Make this an std::array (kinda like RayLib)
    std::vector<Malic::Material> m_materials;
    Malic::VertexArray m_vertexArray;
    Malic::RenderResources m_renderResources;
};

}