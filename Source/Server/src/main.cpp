// main.cpp

#include <stdio.h>
#include "network.h"
#include "core.h"

#include "raylib.h"
#include "application_server.h"
#include "constants_server.h"


int main(int argc, char **argv)
{
    ServerApplication application;

    debug::info("Starting program as Server.");

    const bool bLaunchResults = application.Init();
    
    if (!bLaunchResults)
    {
        return 0;
    }

    while (!WindowShouldClose())
    {
        const float DeltaTime = GetFrameTime();

        application.Update(DeltaTime);
        application.StartRender();
    }

    application.Exit();
    CloseWindow();

    return 0;
}
