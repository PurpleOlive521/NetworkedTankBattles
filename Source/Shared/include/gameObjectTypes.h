#pragma once

#include "core.h"

enum class GameObjectTypes : uint8
{
	None = 0,

	// --- Objects
	GameObject,
	Entity,
	Tank,
	Bullet,

	// --- NonRC Objects
	Wall,
	SpawnPoint,

	// --- Controllers
	Controller,
	EntityController,
	TankController,
	PlayerController,
	BulletController,

	ENUM_MAX
};