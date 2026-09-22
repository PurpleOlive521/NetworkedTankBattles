#pragma once

#include "raylib.h"
#include <vector>
#include "core.h"

class Entity;

enum class ColliderType : uint8
{
	Static,		// Objects that will never themselves be moved.
	Movable,	// Objects that will move.
	OverlapOnly, // Object only wants to be informed of overlaps, and does not affect objects physically.
};

// Represents a set of collider points, relative to a center. 
// This means that the collider needs to be projected onto a point with any present rotation before being used.
struct Collider
{
public:

	// Overwrites the current points with inPoints.
	void SetInitialPoints(std::vector<Vector2>& inPoints);

	// Generates 4 points for the collider, with center being the middlepoint of the collider.
	void GenerateRectangle(Vector2 size, Vector2 center);

	// Returns a copy of this collider, projected over a position rotation.
	Collider Project(Vector2 position, float rotation) const;

	Vector2 GetVelocity() const;

	// The velocity acted upon when stepping the PhysicsEngine.
	void SetVelocity(Vector2 inVelocity);

	ColliderType type = ColliderType::Static;

	std::vector<Vector2> points;

	// Temporarily stop the collider from partaking in physics. We do not receive or generate overlap or physics events.
	bool bDisabled = false;

private:
	Vector2 velocity;
};

struct CollisionEvent
{
	CollisionEvent() = default;

	Entity* other = nullptr;

	// The direction at which we collided with the other object.
	Vector2 direction = {};

	// The point at which the collision occured. This will most likely be inside of the other object, since we dont do sweeping collision calculations.
	Vector2 point = {};

	// The excessive movement that would have been applied if no collision would have occured.
	float penetrationDepth = 0.0f;

	// Normal of the surface we collided with.
	Vector2 normal = {};

};