#include "Client/Application.h"

int main()
{
    Malic::MalicEngine::WindowInfo windowInfo {
        .width = 800,
        .height = 600,
        .title = "Malic Engine"
    };

    MalicClient::Application application(windowInfo);
    application.Run();
    application.ShutDown();

    return 0;
}   