#pragma once

#include <vector>
#include "core.h"

#include "inputManager.h"
#include "gameObjectTypes.h"
#include "level.h"
#include "gameTypes.h"
#include "physicsEngine.h"
#include "timeManager.h"
#include "eventSystem.h"
#include "bulletManager.h"
#include <memory>

class Application;
class GameObject;
class ReplicationManager;

typedef std::unique_ptr<GameObject> GameObjectPtr;

class Game
{
public:

	Game(); 
	
	// Returns true if initialisation was successful.
	bool Init(Application* inOwningApplication);
	void Update(float deltaTime);
	void FixedUpdate(float fixedDeltaTime);
	void Render() const;

	// Adds the GameObject to the game lifecycle to be rendered, updated and managed.
	GameObject* AddGameObject(GameObjectPtr object);

	// Returns a pointer of type T to the newly created object.
	template<typename T>
	T* AddGameObjectByType()
	{
		std::unique_ptr<T> ptr = std::make_unique<T>();
		GameObject* out = AddGameObject(std::move(ptr));
		return static_cast<T*>(out);
	}

	GameObject* CreateAndAddObject(GameObjectTypes inType);
	
	// Returns the time since the game connected to the server for a client, or since the game started if it's a singleplayer game.
	// Synchronised.
	double GetGameTime() const;

	// Synchronised.
	uint32 GetGameTick() const;

	// Synchronised.
	double GetGameTickLength() const;

	ReplicationManager* GetReplicationManager();

	InputManager* GetInputManager();

	TimeManager* GetTimeManager();

	EventSystem* GetEventSystem();

	// Returns nullptr if no level is currently active.
	const Level* GetCurrentLevel() const;

	void AddListener(ObjectListener* inListener);

	void RemoveListener(ObjectListener* inListener);

	void OnGameObjectAdded(GameObject* object);

	void OnGameObjectDestroyed(GameObject* object);

private:

	void RemoveGameObject(GameObject* object);

	// Removes the old level before instantiating the new one.
	void SelectLevel(LevelId inLevel);

	Application* owningApplication = nullptr;

	InputManager inputManager;

	PhysicsEngine physicsEngine;

	TimeManager timeManager;

	EventSystem eventSystem;
	
	BulletManager bulletManager;

	Level* currentLevel = nullptr;

	std::vector<GameObjectPtr> gameObjects;

	std::vector<ObjectListener*> listeners;

	Texture2D groundTexture = {};
};

