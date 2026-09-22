#pragma once

#include <vector>
#include "core.h"

class GameObject;
class SpawnPoint;
class Game;

enum class LevelId : uint8
{
	Basic,

	ENUM_MAX,
};

class Level
{
public:
	Level() = default;
	virtual ~Level() = default;

	virtual void InstantiateLevel(Game* game);

	void DestroyLevel(Game* game);

	LevelId GetLevelId() const;

	// Will return nullptr if no SpawnPoint is present in the level.
	const SpawnPoint* GetRandomSpawnPoint(uint32 rng) const;

protected:
	LevelId id = LevelId::ENUM_MAX;
	
	std::vector<GameObject*> instantiatedObjects;

	std::vector<SpawnPoint*> availableSpawnPoints;
};