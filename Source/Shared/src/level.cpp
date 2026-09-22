#include "level.h"
#include "vector"
#include "gameObject.h"

void Level::InstantiateLevel(Game* game)
{
}

void Level::DestroyLevel(Game* game)
{
	for (auto& object : instantiatedObjects)
	{
		object->SetIsPendingKill(true);
	}

	instantiatedObjects.clear();
}

LevelId Level::GetLevelId() const
{
	return id;
}

const SpawnPoint* Level::GetRandomSpawnPoint(uint32 rng) const
{
	if (availableSpawnPoints.size() == 0)
	{
		return nullptr;
	}

	const int index = (int)rng % availableSpawnPoints.size();

	if (index >= 0 && index < availableSpawnPoints.size())
	{
		return availableSpawnPoints[index];
	}

	return nullptr;
}
