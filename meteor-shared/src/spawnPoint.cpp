#include "spawnPoint.h"

bool SpawnPoint::ShouldUpdate() const
{
	return false;
}

void SpawnPoint::Render()
{
	const Rectangle source = {0.f, 0.f, (float)spawnPointTexture.width, (float)spawnPointTexture.height};
	const Rectangle dest = {GetPosition().x, GetPosition().y, (float)spawnPointTexture.width, (float)spawnPointTexture.height};
	const Vector2 origin = {spawnPointTexture.width * 0.5f,spawnPointTexture.height * 0.5f};
	DrawTexturePro(spawnPointTexture, source, dest, origin, 0.f, WHITE);
}

GameObjectTypes SpawnPoint::GetType() const
{
	return GameObjectTypes::SpawnPoint;
}

bool SpawnPoint::IsPhysicsObject() const
{
	return false;
}
