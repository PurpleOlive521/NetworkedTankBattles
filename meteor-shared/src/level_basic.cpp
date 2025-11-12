#include "level_basic.h"
#include "raylib.h"
#include "wall.h"
#include "game.h"
#include "spawnPoint.h"

Level_Basic::Level_Basic()
{
	id = LevelId::Basic;
}

void Level_Basic::InstantiateLevel(Game* game)
{
	const float screenWidth = (float)GetScreenWidth();
	const float screenHeight = (float)GetScreenHeight();
	

	const std::pair<Vector2, float> walls[] = {
		//Walls Around Spawn Points
		{Vector2{ screenWidth * 0.15f, screenHeight * 0.75f }, 0.f  },
		{Vector2{ screenWidth * 0.22f, screenHeight * 0.86f }, 90.f },
		{Vector2{ screenWidth * 0.85f, screenHeight * 0.25f }, 0.f  },
		{Vector2{ screenWidth * 0.78f, screenHeight * 0.14f }, 90.f },
		{Vector2{ screenWidth * 0.85f, screenHeight * 0.75f }, 0.f  },
		{Vector2{ screenWidth * 0.78f, screenHeight * 0.86f }, 90.f },
		{Vector2{ screenWidth * 0.15f, screenHeight * 0.25f }, 0.f  },
		{Vector2{ screenWidth * 0.22f, screenHeight * 0.14f }, 90.f },

		//Center Walls
		{Vector2{ screenWidth * 0.50f, screenHeight * 0.40f }, 0.f  },
		{Vector2{ screenWidth * 0.50f, screenHeight * 0.60f }, 0.f  },
		{Vector2{ screenWidth * 0.40f, screenHeight * 0.50f }, 90.f },
		{Vector2{ screenWidth * 0.60f, screenHeight * 0.50f },-90.f },

		//Middle Line Walls
		{Vector2{ screenWidth * 0.25f, screenHeight * 0.50f }, 0.f  },
		{Vector2{ screenWidth * 0.75f, screenHeight * 0.50f }, 0.f  },
		{Vector2{ screenWidth * 0.08f, screenHeight * 0.50f }, 90.f },
		{Vector2{ screenWidth * 0.92f, screenHeight * 0.50f },-90.f },
		{Vector2{ screenWidth * 0.50f, screenHeight * 0.15f }, 0.f  },
		{Vector2{ screenWidth * 0.50f, screenHeight * 0.85f }, 0.f  },
	};

	

	for (auto& wall : walls)
	{
		Wall* newWall = game->AddGameObjectByType<Wall>();
		newWall->SetPosition(wall.first);
		newWall->SetRotation(wall.second);
		instantiatedObjects.push_back(newWall);
	}


	const Vector2 spawnPoints[] = {
		Vector2{ screenWidth * 0.15f, screenHeight * 0.15f },
		Vector2{ screenWidth * 0.15f, screenHeight * 0.85f },
		Vector2{ screenWidth * 0.85f, screenHeight * 0.15f },
		Vector2{ screenWidth * 0.85f, screenHeight * 0.85f },
	};

	for (auto& pos : spawnPoints)
	{
		SpawnPoint* newSpawnPoint = game->AddGameObjectByType<SpawnPoint>();
		newSpawnPoint->SetPosition(pos);
		newSpawnPoint->spawnPointTexture = LoadTexture("SpawnPoint.png");
		instantiatedObjects.push_back(newSpawnPoint);
		availableSpawnPoints.push_back(newSpawnPoint);
	}
}
