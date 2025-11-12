#pragma once

#include "entity.h"
#include "physicsTypes.h"

constexpr float BULLET_SPEED = 200.0f;
constexpr int BULLET_RICOCHETS = 2;
 
constexpr float BULLET_WIDTH = 8.0f;
constexpr float BULLET_HEIGHT = 21.0f;

class Tank;

class Bullet : public Entity
{
public:
	Bullet() = default;

	virtual void BeginPlay() override;

	virtual void Update(float deltaTime) override;

	virtual void Render() override;

	virtual GameObjectTypes GetType() const override;

	virtual bool IsPhysicsObject() const override;

	virtual Collider* GetCollider() override;

	virtual void OnCollision(Entity* other, const CollisionEvent& event) override;

	virtual void OnHitTank(Tank* other);

	virtual void OnRicochet(); 

	void UpdatePhysics(float deltaTime);

	// We don't allow collisions with the shooter.
	GameObject* shooter = nullptr;

	int ricochetsLeft = BULLET_RICOCHETS;

private:
	Collider collider = {};

	Texture2D bulletTexture = {};
};