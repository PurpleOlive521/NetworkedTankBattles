#include "Game.h"
#include <assert.h>
#include "gameObject.h"
#include "application.h"
#include "raylib.h"
#include "constants_shared.h"
#include <format>
#include "utility.h"
#include "raymath.h"
#include "levelFactory.h"

Game::Game() : inputManager(this)
{

}

bool Game::Init(Application* inOwningApplication)
{
	assert(inOwningApplication); // No valid application given
	owningApplication = inOwningApplication;

	physicsEngine.Init(this);

	bulletManager.Init(this);

	SelectLevel(LevelId::Basic);

	groundTexture = LoadTexture("GroundLayer.png");
	SetTextureWrap(groundTexture, TEXTURE_WRAP_REPEAT);

	return true;
}

void Game::Update(float deltaTime)
{
	inputManager.Update(deltaTime);

	timeManager.Update(deltaTime);

	physicsEngine.Update(deltaTime);
}

void Game::FixedUpdate(float fixedDeltaTime)
{
	eventSystem.ProcessEventQueue();

	std::vector<GameObject*> objectsToRemove;

	for (auto& object : gameObjects)
	{
		if (object->ShouldUpdate())
		{
			object->Update(fixedDeltaTime);
		}

		if (object->IsPendingKill())
		{
			objectsToRemove.push_back(object.get());
		}
	}

	for (auto& object : objectsToRemove)
	{
		RemoveGameObject(object);
	}

	physicsEngine.Step();
}

void Game::Render() const
{
	const Rectangle sourceRectangle = {0.f, 0.f, (float)groundTexture.width, (float)groundTexture.height};
	
	const Rectangle destRectangle = {0.f, 0.f, (float)GetScreenWidth(), (float)GetScreenHeight()};

	DrawTexturePro(groundTexture, sourceRectangle, destRectangle, { 0,0 }, 0.f, WHITE);

	for (auto& object : gameObjects)
	{
		if (object->IsPendingKill())
		{
			continue;
		}

		object->Render();
	}

	physicsEngine.Render();

	const std::string objectCountText = std::string("GameObjects: ") + std::to_string(gameObjects.size());
	DrawText(objectCountText.c_str(), GetScreenWidth() / 2, 10, 24, RAYWHITE);

	const std::string gameTimeText = std::string("SyncTime: ") + std::format("{:.2f}",GetGameTime());
	DrawText(gameTimeText.c_str(), GetScreenWidth() / 2, 35, 24, RAYWHITE);

	const std::string gameTickText = std::string("Tick: ") + std::to_string(GetGameTick());
	DrawText(gameTickText.c_str(), GetScreenWidth() / 2, 60, 24, RAYWHITE);
}

GameObject* Game::AddGameObject(GameObjectPtr object)
{
	assert(object);

	GameObject* out = object.get();
	gameObjects.push_back(std::move(object));

	out->PreBeginPlay(this);
	out->BeginPlay();

	OnGameObjectAdded(out);

	return out;
}

GameObject* Game::CreateAndAddObject(GameObjectTypes inType)
{
	return owningApplication->CreateAndAddObject(inType);
}

double Game::GetGameTime() const
{
	return GetTime() + owningApplication->GetSynchronizationTime();
}

uint32 Game::GetGameTick() const
{
	return (uint32)round(GetGameTime() / meteor::sharedConstants::TICK_RATE);
}

double Game::GetGameTickLength() const
{
	return meteor::sharedConstants::TICK_RATE;
}

ReplicationManager* Game::GetReplicationManager()
{
	return owningApplication->GetReplicationManager();
}

InputManager* Game::GetInputManager()
{
	return &inputManager;
}

TimeManager* Game::GetTimeManager()
{
	return &timeManager;
}

EventSystem* Game::GetEventSystem()
{
	return &eventSystem;
}

const Level* Game::GetCurrentLevel() const
{
	if (currentLevel)
	{
		return currentLevel;
	}

	return nullptr;
}

void Game::AddListener(ObjectListener* inListener)
{
	assert(inListener);

	for (int i = 0; i < listeners.size(); i++)
	{
		if (listeners[i] == inListener)
		{
			return;
		}
	}

	listeners.push_back(inListener);
}

void Game::RemoveListener(ObjectListener* inListener)
{
	assert(inListener);

	for (int i = 0; i < listeners.size(); i++)
	{
		if (listeners[i] == inListener)
		{
			listeners.erase(listeners.begin() + i);
			return;
		}
	}
}

void Game::OnGameObjectAdded(GameObject* object)
{
	for (auto& listener : listeners)
	{
		listener->OnGameObjectCreated(object);
	}
}

void Game::OnGameObjectDestroyed(GameObject* object)
{
	for (auto& listener : listeners)
	{
		listener->OnGameObjectDestroyed(object);
	}
}

void Game::RemoveGameObject(GameObject* object)
{
	auto it = std::find_if(gameObjects.cbegin(), gameObjects.cend(), [object](const GameObjectPtr& other)
		{
			return other.get() == object;
		}
	);

	if (it != gameObjects.cend())
	{
		OnGameObjectDestroyed(object);
		object->OnDestroy();

		gameObjects.erase(it);
	}
}

void Game::SelectLevel(LevelId inLevel)
{
	if (currentLevel)
	{
		currentLevel->DestroyLevel(this);
	}
	
	currentLevel = LevelFactory::CreateLevel(inLevel);
	currentLevel->InstantiateLevel(this);
}

