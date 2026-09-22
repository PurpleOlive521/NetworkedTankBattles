#include "application_client.h"

#include "Raylib.h"
#include "raymath.h"
#include "client.h"
#include "constants_shared.h"

using namespace meteor;

ClientApplication::ClientApplication() : replicationManager(&game)
{
}

bool ClientApplication::Init()
{
    const int screenWidth = sharedConstants::SCREEN_WIDTH;
    const int screenHeight = sharedConstants::SCREEN_HEIGHT;

	const char* WindowTitle = "Meteor - Client";

    InitWindow(screenWidth, screenHeight, WindowTitle);

    // Initialise late, in case that Application depends on window state
    Application::Init();
    client.SetListener(&replicationManager);

    SetExitKey(KEY_ESCAPE);

    const bool bInitSuccess = client.Init(this);

    if (!bInitSuccess)
    {
        debug::info("Program did not initialise correctly. Terminating.");
        return false;
    }

    return true;
}

void ClientApplication::Update(float deltaTime)
{
     client.Update(deltaTime);
     replicationManager.Update(deltaTime);
     Application::Update(deltaTime);
}

void ClientApplication::Render() const
{
    Application::Render();

    replicationManager.Render();

    client.Render();
}

void ClientApplication::Exit()
{
    client.Exit();
}

double ClientApplication::GetSynchronizationTime()
{
    return client.GetConnectionTimestamp();
}

ClientReplicationManager* ClientApplication::GetClientReplicationManager()
{
    return &replicationManager;
}

ReplicationManager* ClientApplication::GetReplicationManager()
{
    return &replicationManager;
}

