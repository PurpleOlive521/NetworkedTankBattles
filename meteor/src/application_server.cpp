#include "application_server.h"

#include "Raylib.h"
#include "server.h"
#include "constants_shared.h"

using namespace meteor;

ServerApplication::ServerApplication() : replicationManager(&game)
{
}

bool ServerApplication::Init()
{
    const int screenWidth = sharedConstants::SCREEN_WIDTH;
    const int screenHeight = sharedConstants::SCREEN_HEIGHT;

	const char* WindowTitle = "Meteor - Server";

    InitWindow(screenWidth, screenHeight, WindowTitle);

    // Initialise late, in case that Application depends on window state
    Application::Init();
    server.SetListener(&replicationManager);

    SetExitKey(KEY_ESCAPE);

    const bool bInitSuccess = server.Init(this);
    
    if (!bInitSuccess)
    {
        debug::info("Program did not initialise correctly. Terminating.");
        return false;
    }

    return true;
}

void ServerApplication::Update(float deltaTime)
{
    // We want to receive messages before we update game
    server.Update(deltaTime);
    replicationManager.Update(deltaTime);
    Application::Update(deltaTime);
}

void ServerApplication::Render() const
{
	// Allow game and other application rendering, while server should only render debug info & overlays.
    Application::Render();

    replicationManager.Render();

    server.Render();
}

void ServerApplication::Exit()
{
    server.Exit();
}

ServerReplicationManager* ServerApplication::GetServerReplicationManager()
{
    return &replicationManager;
}

GameObject* ServerApplication::CreateAndAddObject(GameObjectTypes inType)
{
    return replicationManager.CreateAndAddObject(inType);
}

ReplicationManager* ServerApplication::GetReplicationManager()
{
    return &replicationManager;
}



