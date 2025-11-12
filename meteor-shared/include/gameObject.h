#pragma once

#include "core.h"
#include "constants_shared.h"
#include "gameObjectTypes.h"

class Game;

class GameObject
{
public:
	GameObject() = default;
	virtual ~GameObject();

	// --- Object Lifecycle

	// Does some boilerplate setup for the object.
	void PreBeginPlay(Game* CreationGame);

	// Called when the object is first created. Only called when added to a game world.
	virtual void BeginPlay();

	// Return true if the object wants to be updated this frame.
	virtual bool ShouldUpdate() const;

	// Called every frame.
	virtual void Update(float DeltaTime);

	virtual void Render();

	virtual bool IsPendingKill() const;
	
	void SetIsPendingKill(bool bInState);

	// Always called just before the object is destroyed. Guaranteed to be called only once.
	virtual void OnDestroy();

	virtual GameObjectTypes GetType() const;

protected:

	bool bPendingKill = false;

	// The game that owns this object.
	Game* owningGame = nullptr;
};

