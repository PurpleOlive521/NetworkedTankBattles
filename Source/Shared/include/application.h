#pragma once

#include "network_layer.h"
#include "game.h"
#include "timer.h"
#include "constants_shared.h"

class GameObjectFactory;
class ReplicationManager;

// Abstract base class to build the application lifecycle from, which will in turn handle a server or client.
class Application
{
public:
	Application() = default;

	virtual bool Init();

	virtual void Update(float deltaTime);

	// Preps and ends the window for us, rather than needing to keep track of it's state throughout Render() overrides.
	void StartRender();

	// Do not call directly, use StartRender instead.
	virtual void Render() const;

	virtual void Exit();

	virtual double GetSynchronizationTime();

	virtual GameObject* CreateAndAddObject(GameObjectTypes inType);

	virtual ReplicationManager* GetReplicationManager();

	Game* GetGame();

protected:

	Game game;

	Timer gameUpdateTimer = { meteor::sharedConstants::GAME_UPDATE_RATE };

	// Constructor starts up winsock
	meteor::network::startup netboot;
};