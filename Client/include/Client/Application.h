#pragma once

#include <memory>

#include "Engine/Malic.h"

#include "Client/Core.h"

namespace MalicClient
{

class Application
{
public:
    Application(Malic::MalicEngine::WindowInfo window_info);
    ~Application() = default;

    void Run();
    void ShutDown();

private:
    std::unique_ptr<ApplicationData> m_applicationData;
    Malic::MalicEngine m_engine;
};
    
}