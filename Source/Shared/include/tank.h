#pragma once

#include "entity.h"
#include "physicsTypes.h"
#include "timeManager.h"

constexpr float TANK_ACCELERATION = 100.0f;
constexpr float TANK_MAX_SPEED = 100.0f;

constexpr float TANK_TURN_RATE = 180.0f;
constexpr float TANK_MAX_TURN_SPEED = 90.0f;

constexpr float TANK_TURRET_TURN_RATE = 135.0f;

constexpr float TANK_FRICTION = 0.90f;
constexpr float TANK_ROTATION_FRICTION = 0.85f;
constexpr float LOWER_EPSILON = 0.01f;

constexpr float TANK_RESPAWN_TIME = 3.0f;
constexpr float TANK_RELOAD_TIME = 1.0f;

constexpr float TANK_BODY_WIDTH = 25.0f;
constexpr float TANK_BODY_HEIGHT = 49.0f;

class Tank: public Entity
{
public:
	Tank();

	virtual void BeginPlay() override;

	virtual void Update(float deltaTime) override;

	virtual void Render() override;

	virtual GameObjectTypes GetType() const override;

	virtual bool IsPhysicsObject() const override;

	virtual Collider* GetCollider() override;
	
	void UpdatePhysics(float deltaTime);

	void Accelerate();
	
	void Reverse();

	// Relative to the forward vector.
	void TurnLeft();

	// Relative to the forward vector.
	void TurnRight();

	void Aim(Vector2 target);

	// Returns true if the tank successfully shoots.
	virtual bool TryShoot();

	void Reload();

	// Starts the death sequence for the tank.
	void TriggerDeath();

	void Respawn();

	float turretRotation = 0.0f;
	Vector2 aimTarget = {};

	Collider collider;

	float pendingSpeed = 0.0f;
	float forwardSpeed = 0.0f;

	float pendingTurn = 0.0f;
	float turnSpeed = 0.0f;

	TimerHandle respawnTimerHandle;
	TimerHandle reloadTimerHandle;

	int availableBullets = 1;
	bool bIsDead = false;

	Texture2D tankBaseTexture = {};
	Texture2D tankTurretTexture = {};
};