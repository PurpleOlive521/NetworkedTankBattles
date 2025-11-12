// main.cpp

#include <stdio.h>
#include "network.h"

#include "raylib.h"
#include "application_client.h"
#include "constants_client.h"


int main(int argc, char **argv)
{
    ClientApplication application;

    debug::info("Starting program as Client.");

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
