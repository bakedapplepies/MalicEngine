#include "Client/Model.h"

#include <filesystem>
#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Engine/core/Assert.h"

namespace MalicClient
{

Model::Model(const Malic::MalicEngine* engine, const char* path)
{
    Assimp::Importer importer;
    // float time = glfwGetTime();
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipWindingOrder);
    bool failedResult = !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode;
    MLC_ASSERT(!failedResult, fmt::format("Failed to load model [{}] | {}", path, importer.GetErrorString()));

    std::vector<Malic::Vertex> vertices;
    std::vector<uint16_t> indices;

    _GetMaterials(scene, engine);
    _ProcessNode(scene->mRootNode, scene, vertices, indices);
    // fmt::print("{}\n", glfwGetTime() - time);
}

Model::~Model()
{
    m_materials.clear();
}

void Model::_GetMaterials(const aiScene* scene, const Malic::MalicEngine* engine)
{
    const Malic::ResourceManager* resourceManager = engine->GetResourceManager();

    m_materials.reserve(scene->mNumMaterials);
    for (uint32_t i = 0; i < scene->mNumMaterials; i++)
    {
        Malic::Material mlcMaterial;
        aiMaterial* material = scene->mMaterials[i];
        aiString path;
        
        aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &path);
        std::filesystem::path actualPath = "Client/resources/models/vivian";
        actualPath /= path.C_Str();
        std::u32string actuallPath = actualPath.u32string();
        std::replace(actuallPath.begin(), actuallPath.end(), '\\', '/');
        // fmt::print(u"{}\n", actuallPath.c_str());
        mlcMaterial.SetAlbedo(resourceManager->GetTexture2D(Malic::File(actuallPath.c_str())));
    }
}

void Model::_ProcessNode(const aiNode* node,
                         const aiScene* scene,
                         std::vector<Malic::Vertex>& vertices,
                         std::vector<uint16_t>& indices) const
{
    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* currentMesh = scene->mMeshes[node->mMeshes[i]];
        _ProcessMesh(currentMesh, vertices, indices);
    }
    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        _ProcessNode(node->mChildren[i], scene, vertices, indices);
    }
}

void Model::_ProcessMesh(const aiMesh* mesh,
                         std::vector<Malic::Vertex>& vertices,
                         std::vector<uint16_t>& indices) const
{
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        glm::vec3 position(mesh->mVertices[i].x,
                           mesh->mVertices[i].y,
                           mesh->mVertices[i].z);
        glm::vec3 color(1.0f, 1.0f, 1.0f);
        glm::vec2 uv;
        if (mesh->mTextureCoords[0])
        {
            uv = glm::vec2(mesh->mTextureCoords[0][i].x,
                           mesh->mTextureCoords[0][i].y);
        }
        else { uv = glm::vec2(0.0f, 0.0f); }

        vertices.push_back(Malic::Vertex {
            .position = position,
            .color = color,
            .uv = uv
        });

    }
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace& face = mesh->mFaces[i];
        for (uint16_t j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }
}

}