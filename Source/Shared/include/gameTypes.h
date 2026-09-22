#pragma once

class GameObject;

class ObjectListener
{
public:
	// Called after the objects BeginPlay.
	virtual void OnGameObjectCreated(GameObject* object) = 0;

	// Called before the objects OnDestroy.
	virtual void OnGameObjectDestroyed(GameObject* object) = 0;
};