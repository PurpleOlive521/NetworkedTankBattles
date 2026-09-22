#pragma once

#include "entity.h"

class SpawnPoint final : public Entity
{
public:
	SpawnPoint() = default;

	bool ShouldUpdate() const override;

	void Render() override;

	GameObjectTypes GetType() const override;

	bool IsPhysicsObject() const override;

	Texture2D spawnPointTexture;
};