#include "Client/Application.h"

#include <filesystem>
#include <vector>
#include <memory>
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
#include "Engine/Texture2D.h"

#include "Client/Input.h"

namespace MalicClient
{

struct alignas(16) MVP_UBO
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

Application::Application(Malic::MalicEngine::WindowInfo window_info)
    : m_engine(window_info)
{}

void Application::Run()
{
    using namespace MalicClient;

    m_applicationData = ApplicationData {
        .camera = MalicClient::Camera(
            glm::vec3(0.0f, 0.0f, 1.0f),   // position
            glm::vec3(0.0f, 0.0f, -1.0f),  // direction
            0.1f,                          // near
            10.0f,                         // far
            45.0f                          // fov in degrees
        )
    };
    m_engine.SetUserPointer(&m_applicationData);
    m_engine.Run();
}

void Application::ShutDown()
{
    m_applicationData = ApplicationData {};
    m_engine.ShutDown();
}

}

void MalicEntry(Malic::MalicEngine* engine)
{
    using namespace MalicClient;

    engine->HideCursor();

    ApplicationData* myData = static_cast<ApplicationData*>(engine->GetUserPointer());
    Malic::VulkanManager* vulkanManager = engine->GetMutVulkanManager();
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

    myData->vertexArrays.emplace_back(
        vulkanManager,
        0,  // binding
        vertices,
        indices
    );

    std::filesystem::path vertPath = "../../Engine/resources/shaders/bin/default_vert.spv";
    std::filesystem::path fragPath = "../../Engine/resources/shaders/bin/default_frag.spv";
    Malic::Shader defaultShader(
        vulkanManager,
        vertPath.string(),
        fragPath.string()
    );

    vulkanManager->CreateDescriptorSetLayout();
    vulkanManager->CreateDescriptorSets();

    Malic::VulkanManager::GraphicsPipelineConfig pipelineConfig
    {
        .shader = &defaultShader,
        .vertexArrays = &myData->vertexArrays
    };
    vulkanManager->CreateGraphicsPipeline(pipelineConfig);

    myData->uniformBuffer = std::make_unique<Malic::UniformBuffer>(vulkanManager, 0, sizeof(MVP_UBO));

    myData->texture = std::make_unique<Malic::Texture2D>(vulkanManager, "../../Engine/resources/images/hamster.jpg");
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

    myData->uniformBuffer->UpdateData(&mvpUBOData, sizeof(MVP_UBO));
}