#pragma once

#include "gameTypes.h"
#include <vector>
#include "physicsTypes.h"

class Game;
class Entity;

// Used internally to track and store data about a collision.
struct CollisionData
{
	CollisionData() = default;

	CollisionData(Entity* self, Entity* other);

	Entity* self = nullptr;

	Entity* other = nullptr;

	Vector2 mtv = {};

	Vector2 collidedNormal = {};

	float depth = 0.0f;
};

class PhysicsEngine final : public ObjectListener
{
public:
	PhysicsEngine() = default;

	// --- Begin ObjectListener Interface
	void OnGameObjectCreated(GameObject* object) override;

	void OnGameObjectDestroyed(GameObject* object) override;
	// --- End ObjectListener Interface

	void Init(Game* game);

	void Update(float deltaTime);

	void Render() const;

	// Returns true if the objects are overlapping.
	bool AreOverlapping(Entity* self, Entity* other, CollisionData& outCollisionData);	
	
	void Step();

	bool GetDebugMode() const;

	CollisionEvent ResolveCollision(const CollisionData& collisionEvent);

private:
	// First is min, second is max.
	typedef std::pair<float, float> ProjectionRange;

	void GetColliderNormals(const Collider& collider, std::vector<Vector2>& normals);

	ProjectionRange GetProjection(const Collider& collider, Vector2 normal);

	Game* owningGame = nullptr;

	std::vector<Entity*> physicsEntities;

	bool drawDebug = false;
};