#pragma once

#include "gameObject.h"
#include "raylib.h"
#include "physicsTypes.h"

constexpr Vector2 DEFAULT_DIRECTION = { 0, 1 };

class Entity : public GameObject
{
public:
	Entity() = default;

	virtual void Update(float deltaTime) override;

	virtual void Render() override;

	virtual GameObjectTypes GetType() const override;

	virtual void OnDestroy() override;

	// Begin Collision Logic 
	
	// If false, the object won't partake in the physics system.
	virtual bool IsPhysicsObject() const;

	virtual Collider* GetCollider();

	virtual void OnCollision(Entity* other, const CollisionEvent& event);
	// Begin Collision Logic 


	void SetPosition(Vector2 inPosition);

	Vector2 GetPosition() const;

	void SetRotation(float inRotation);

	float GetRotation() const;

	Vector2 GetForwardVector() const;

	// The controller of this Entity. Can be nullptr if the object is not a type supported by controllers.
	GameObject* controller = nullptr;

private:
	Vector2 forwardVector = DEFAULT_DIRECTION;
	
	Vector2 position = {};

	float rotation = 0.0f;

};