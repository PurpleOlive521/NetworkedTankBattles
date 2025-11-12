#include "application.h"
#include "core.h"
#include "constants_shared.h"
#include "raylib.h"

bool Application::Init() 
{ 
	bool bSuccess = true;

	bSuccess &= game.Init(this);

	if (!bSuccess)
	{
		debug::error("Application failed to initialise.");
		return false;
	}

	return true; 
}

void Application::Update(float deltaTime)
{
	game.Update(deltaTime);

	if (gameUpdateTimer.IsFinished())
	{
		game.FixedUpdate(meteor::sharedConstants::GAME_UPDATE_RATE);
		gameUpdateTimer.Restart();
	}
}

void Application::StartRender()
{
	BeginDrawing();
	ClearBackground(RAYWHITE);

	Render();

	EndDrawing();
}

void Application::Render() const
{
	game.Render();
}

void Application::Exit() 
{

}

double Application::GetSynchronizationTime()
{
	return 0.0;
}

GameObject* Application::CreateAndAddObject(GameObjectTypes inType)
{
	return nullptr;
}

ReplicationManager* Application::GetReplicationManager()
{
	return nullptr;
}

Game* Application::GetGame()
{
	return &game;
}