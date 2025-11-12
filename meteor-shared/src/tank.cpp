#include "tank.h"
#include "raymath.h"
#include "utility.h"
#include "game.h"
#include "spawnPoint.h"
#include "replicationManager.h"

Tank::Tank()
{
	collider.GenerateRectangle(Vector2{ TANK_BODY_WIDTH, TANK_BODY_HEIGHT }, Vector2{});
	collider.type = ColliderType::Movable;
}

void Tank::BeginPlay()
{
	Entity::BeginPlay();

	tankBaseTexture = LoadTexture("Enemies/EnemyBase.png");

	tankTurretTexture = LoadTexture("Enemies/EnemyTurret.png");

	Respawn();
}

void Tank::Update(float deltaTime)
{
	UpdatePhysics(deltaTime);
}

void Tank::Render()
{
	if (bIsDead)
	{
		return;
	}

	const Rectangle bodyRectangle = {GetPosition().x, GetPosition().y, TANK_BODY_WIDTH, TANK_BODY_HEIGHT };
	const Rectangle textureBaseRectangle = {0, 0, TANK_BODY_WIDTH, TANK_BODY_HEIGHT };

	DrawTexturePro(tankBaseTexture, textureBaseRectangle, bodyRectangle, GetRectCenter(bodyRectangle), GetRotation(), WHITE);

	const float turretWidth = 17.0f;
	const float turretHeight = 56.0f;
	const Rectangle turretRectangle = {GetPosition().x, GetPosition().y, turretWidth, turretHeight};
	const Rectangle textureTurretRectangle = {0, 0, turretWidth, turretHeight};
	Vector2 origin = {GetRectCenter(turretRectangle).x, GetRectCenter(turretRectangle).y + 13};
	DrawTexturePro(tankTurretTexture, textureTurretRectangle, turretRectangle, origin, turretRotation, WHITE);
}

GameObjectTypes Tank::GetType() const
{
	return GameObjectTypes::Tank;
}

bool Tank::IsPhysicsObject() const
{
	return true;
}

Collider* Tank::GetCollider()
{
	return &collider;
}

void Tank::UpdatePhysics(float deltaTime)
{
	if (bIsDead)
	{
		return;
	}

	// --- Movement
	forwardSpeed += pendingSpeed;
	turnSpeed += pendingTurn;

	forwardSpeed = Clamp(forwardSpeed, -TANK_MAX_SPEED, TANK_MAX_SPEED);
	turnSpeed = Clamp(turnSpeed, -TANK_MAX_TURN_SPEED, TANK_MAX_TURN_SPEED);

	forwardSpeed *= TANK_FRICTION;
	turnSpeed *= TANK_ROTATION_FRICTION;

	Vector2 movement = Vector2Scale(GetForwardVector(), forwardSpeed * deltaTime);
	collider.SetVelocity(movement);

	// --- Rotation

	SetRotation(GetRotation() + turnSpeed * deltaTime);

	// --- Turret

	Vector2 aimTargetDirection = Vector2Normalize(GetPosition() - aimTarget);

	const float desiredTurretRotation = ToRayDeg(180.0f + Atan2Degrees(aimTargetDirection.y, aimTargetDirection.x));
	
	int shortestRotation = GetShortestRotation(turretRotation, desiredTurretRotation);

	if (fabs(shortestRotation) > LOWER_EPSILON)
	{
		int direction = shortestRotation > 0 ? 1 : -1;

		float sumRotation = TANK_TURRET_TURN_RATE * direction * deltaTime;

		// Snap to the desired angle if we would rotate past it, otherwise just apply the rotation.
		if (fabs(sumRotation) < fabs(shortestRotation))
		{
			turretRotation += sumRotation;
		}
		else
		{
			turretRotation = desiredTurretRotation;
		}
	}
	else 
	{
		// Difference is tiny, snap to it
		turretRotation += shortestRotation;
	}

	// --- Clamping

	if (fabs(forwardSpeed) < LOWER_EPSILON)
	{
		forwardSpeed = 0.0f;
	}
	if (fabs(turnSpeed) < LOWER_EPSILON)
	{
		turnSpeed = 0.0f;
	}

	if (turretRotation > 360.0f)
	{
		turretRotation -= 360.0f;
	}
	if (turretRotation < 0.0f)
	{
		turretRotation += 360.0f;
	}

	pendingSpeed = 0.0f;
	pendingTurn = 0.0f;
}

void Tank::Accelerate()
{
	pendingSpeed += TANK_ACCELERATION;
}

void Tank::Reverse()
{
	pendingSpeed -= TANK_ACCELERATION;
}

void Tank::TurnLeft()
{
	pendingTurn -= TANK_TURN_RATE;
}

void Tank::TurnRight()
{
	pendingTurn += TANK_TURN_RATE;
}

void Tank::Aim(Vector2 target)
{
	aimTarget = target;
}

bool Tank::TryShoot()
{
	if (bIsDead)
	{
		return false;
	}

	if (availableBullets > 0)
	{
		availableBullets--;

		TimerDelegate reloadTimer = std::bind(&Tank::Reload, this);
		reloadTimerHandle = owningGame->GetTimeManager()->StartTimer(reloadTimer, TANK_RELOAD_TIME);

		return true;
	}

	return false;
}

void Tank::Reload()
{
	availableBullets++;
}

void Tank::TriggerDeath()
{
	if (bIsDead)
	{
		return;
	}

	collider.bDisabled = true;
	bIsDead = true;

	
	TimerDelegate respawnTimer = std::bind(&Tank::Respawn, this);
	respawnTimerHandle = owningGame->GetTimeManager()->StartTimer(respawnTimer, TANK_RESPAWN_TIME);
}

void Tank::Respawn()
{
	collider.bDisabled = false;
	bIsDead = false;

	const uint32 ourId = owningGame->GetReplicationManager()->GetNetworkIdOfObject(this);
	const SpawnPoint* chosenSpawnPoint = owningGame->GetCurrentLevel()->GetRandomSpawnPoint(ourId);
	if (chosenSpawnPoint)
	{
		SetPosition(chosenSpawnPoint->GetPosition());
	}
}
