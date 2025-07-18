#include "Client/Application.h"

#include <vector>
#include <fmt/format.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Engine/MalicEntry.h"
#include "Engine/VertexArray.h"
#include "Engine/Shader.h"
#include "Engine/UniformBuffer.h"
#include "Engine/Material.h"

#include "Client/Input.h"
#include "Client/Model.h"

namespace MalicClient
{

struct alignas(16) MVP_UBO
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct PushConstants
{
};
static_assert(sizeof(PushConstants) <= Malic::MAX_PUSH_CONSTANTS_SIZE, "PushConstants size larger than 128 bytes.");

Application::Application(Malic::MalicEngine::WindowInfo window_info)
    : m_engine(window_info)
{}

void Application::Run()
{
    m_applicationData = std::make_unique<ApplicationData>();
    m_applicationData->camera = MalicClient::Camera(
        glm::vec3(0.0f, 0.0f, 1.0f),   // position
        glm::vec3(0.0f, 0.0f, -1.0f),  // direction
        0.1f,                          // near
        10.0f,                         // far
        45.0f                          // fov in degrees
    );
    m_engine.SetUserPointer(static_cast<void*>(m_applicationData.get()));
    m_engine.Run();
}

void Application::ShutDown()
{
    m_applicationData.reset();
    m_engine.ShutDown();
}

}

void MalicEntry(Malic::MalicEngine* engine)
{
    using namespace MalicClient;

    engine->HideCursor();

    const Malic::ResourceManager* resourceManager = engine->GetResourceManager();
    ApplicationData* myData = static_cast<ApplicationData*>(engine->GetUserPointer());

    const std::vector<Malic::Vertex> vertices {
        Malic::Vertex {
            .position = { -0.5f, 0.5f, 0.0f },
            .color = { 1.0f, 0.0f, 0.0f },
            .uv = { 0.0f, 0.0f }
        },
        Malic::Vertex {
            .position = { 0.5f, 0.5f, 0.0f },
            .color = { 0.0f, 1.0f, 0.0f },
            .uv = { 1.0f, 0.0f }
        },
        Malic::Vertex {
            .position = { 0.5f, -0.5f, 0.0f },
            .color = { 0.0f, 0.0f, 1.0f },
            .uv = { 1.0f, 1.0f }
        },
        Malic::Vertex {
            .position = { -0.5f, -0.5f, 0.0f },
            .color = { 1.0f, 1.0f, 1.0f },
            .uv = { 0.0f, 1.0f }
        },
        Malic::Vertex {
            .position = { -0.5f, 0.5f, 1.0f },
            .color = { 1.0f, 0.0f, 0.0f },
            .uv = { 0.0f, 0.0f }
        },
        Malic::Vertex {
            .position = { 0.5f, 0.5f, 1.0f },
            .color = { 0.0f, 1.0f, 0.0f },
            .uv = { 1.0f, 0.0f }
        },
        Malic::Vertex {
            .position = { 0.5f, -0.5f, 1.0f },
            .color = { 0.0f, 0.0f, 1.0f },
            .uv = { 1.0f, 1.0f }
        },
        Malic::Vertex {
            .position = { -0.5f, -0.5f, 1.0f },
            .color = { 1.0f, 1.0f, 1.0f },
            .uv = { 0.0f, 1.0f }
        }
    };
    const std::vector<uint16_t> indices {
        0, 1, 2,
        0, 2, 3,
        4, 5, 6,
        4, 6, 7
    };

    myData->vertexArrays.push_back(engine->CreateVertexArray(vertices, indices));
    myData->vertexArrays.push_back(engine->CreateVertexArray(vertices, indices));
    
    Malic::File vertFile("Client/resources/shaders/bin/default_vert.spv");
    Malic::File fragFile("Client/resources/shaders/bin/default_frag.spv");
    Malic::Shader defaultShader = resourceManager->GetShader(
        vertFile,
        fragFile
    );

    std::vector<Malic::DescriptorInfo> descriptorInfos;
    descriptorInfos.push_back(Malic::DescriptorInfo {
        .type = Malic::DescriptorTypes::UNIFORM_BUFFER,
        .stageFlags = Malic::ShaderStages::VERTEX_BIT,
        .binding = 0,
        .count = 1
    });
    descriptorInfos.push_back(Malic::DescriptorInfo {
        .type = Malic::DescriptorTypes::COMBINED_IMAGE_SAMPLER,
        .stageFlags = Malic::ShaderStages::FRAGMENT_BIT,
        .binding = 1,
        .count = 1
    });
    engine->CreateDescriptors(descriptorInfos);
    
    Malic::Material material(defaultShader);
    material.SetAlbedo(resourceManager->GetTexture2D(Malic::File("Client/resources/models/vivian/tex/颜.png")));
    Malic::PipelineResources pipelineConfig
    {
        .material = material,
        .vertexInputBindingDescs = myData->vertexArrays.at(0).GetBindingDescriptions(),
        .vertexInputAttribDescs = myData->vertexArrays.at(0).GetAttribDescriptions()
    };
    engine->AssignPipeline(pipelineConfig);
    engine->AssignRenderList({
        Malic::RenderResources {
            .material = material,
            .vertexArray = &myData->vertexArrays.at(0)
        }
    });

    myData->uniformBuffer = engine->CreateUBO(0, sizeof(MVP_UBO));

    // Model model(engine, "../../Client/resources/models/vivian/vivian.pmx");
}

void MalicUpdate(Malic::MalicEngine* engine, float delta_time)
{
    using namespace MalicClient;

    static ApplicationData* myData = static_cast<ApplicationData*>(engine->GetUserPointer());
    const Malic::MalicEngine::WindowInfo* windowInfo = engine->GetWindowInfo();

    MalicClient::ProcessInput(engine, delta_time);

    MVP_UBO mvpUBOData;

    glm::mat4 model = glm::mat4(1.0f);
    // model = glm::rotate(model,
    //                     static_cast<float>(glfwGetTime()) * glm::radians(10.0f),
    //                     glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 view = myData->camera.GetViewMat();
    glm::mat4 projection =
        myData->camera.GetProjMat(static_cast<float>(windowInfo->width)/static_cast<float>(windowInfo->height));

    mvpUBOData.model = model;
    mvpUBOData.view = view;
    mvpUBOData.projection = projection;

    myData->uniformBuffer.UpdateData(&mvpUBOData, sizeof(MVP_UBO));
}