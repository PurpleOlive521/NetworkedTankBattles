#include "physicsEngine.h"
#include "core.h"
#include "entity.h"
#include "raylib.h"
#include "physicsTypes.h"
#include "game.h"
#include "utility.h"
#include "raymath.h"

CollisionData::CollisionData(Entity* inSelf, Entity* inOther)
{
	assert(inSelf);
	assert(inOther);

	self = inSelf;
	other = inOther;
}

void PhysicsEngine::OnGameObjectCreated(GameObject* object)
{
	Entity* asEntity = dynamic_cast<Entity*>(object);

	if (asEntity && asEntity->IsPhysicsObject())
	{
		physicsEntities.emplace_back(asEntity);
	}
}

void PhysicsEngine::OnGameObjectDestroyed(GameObject* object)
{
	auto it = std::find(physicsEntities.begin(), physicsEntities.end(), object);

	if (it != physicsEntities.end())
	{
		physicsEntities.erase(it);
	}
}

void PhysicsEngine::Init(Game* game)
{
	assert(game);
	owningGame = game;
	owningGame->AddListener(this);
}

void PhysicsEngine::Update(float deltaTime)
{
	if (IsKeyPressed(KEY_P))
	{
		drawDebug = !drawDebug;
	}
}

void PhysicsEngine::Render() const
{
	if (!drawDebug)
	{
		return;
	}

	for (auto& entity : physicsEntities)
	{
		const Collider* collider = entity->GetCollider();
		const Collider drawCollider = collider->Project(entity->GetPosition(), entity->GetRotation());
		for (int i = 0; i < drawCollider.points.size(); i++)
		{
			DrawLineEx(drawCollider.points[i], drawCollider.points[(i + 1) % drawCollider.points.size()], 2.0f, PURPLE);
		}
	}
}

bool PhysicsEngine::AreOverlapping(Entity* self, Entity* other, CollisionData& outCollisionData)
{
	std::vector<Vector2> normals;

	Collider selfCollider = self->GetCollider()->Project(self->GetPosition(), self->GetRotation());
	Collider otherCollider = other->GetCollider()->Project(other->GetPosition(), other->GetRotation());

	GetColliderNormals(selfCollider, normals);
	GetColliderNormals(otherCollider, normals);

	Vector2 depthAxis = {};
	float smallestDepth = FLT_MAX;

	//Project both colliders onto each axis
	for (Vector2& axis : normals)
	{
		ProjectionRange selfRange = GetProjection(selfCollider, axis);
		ProjectionRange otherRange = GetProjection(otherCollider, axis);

		//Inequality formula
		bool selfInsideother = selfRange.first < otherRange.second && selfRange.first > otherRange.first;
		bool otherInsideself = otherRange.first < selfRange.second && otherRange.first > selfRange.first;

		if (selfInsideother || otherInsideself)
		{
			// Calculate MTV in case the objects need to be separated
			const float maxLowest = fmax(selfRange.first, otherRange.first);
			const float minHighest = fmin(selfRange.second, otherRange.second);
			float axisDepth = fmax(maxLowest, minHighest) - fmin(maxLowest, minHighest);

			if (axisDepth < smallestDepth)
			{
				depthAxis = axis;
				smallestDepth = axisDepth;
			}

			continue;
		}
		else // No collision on one axis, no overlap
		{
			return false;
		}
	}

	outCollisionData.self = self;
	outCollisionData.other = other;
	outCollisionData.mtv = depthAxis * smallestDepth * Vector2{ 1,-1 }; // Raylib coordinate system
	outCollisionData.depth = smallestDepth;
	outCollisionData.collidedNormal = depthAxis;

	//All axis have overlap, therefore a overlap is present
	return true;
}

void PhysicsEngine::Step()
{
	std::vector<CollisionData> collisions;

	for (auto& outer : physicsEntities)
	{
		// Apply each entity's force accumulated this frame
		outer->SetPosition(outer->GetPosition() + outer->GetCollider()->GetVelocity());

		for (auto& inner : physicsEntities)
		{
			if (outer->GetCollider()->bDisabled || inner->GetCollider()->bDisabled)
			{
				continue;
			}

			if (outer == inner)
			{
				continue;
			}

			CollisionData data;

			if (AreOverlapping(outer, inner, data))
			{
				collisions.emplace_back(data);
			}
		}
	}

	for (auto& collision : collisions)
	{
		CollisionEvent event = ResolveCollision(collision);
		collision.self->OnCollision(collision.other, event);
	}
}

bool PhysicsEngine::GetDebugMode() const
{
	return drawDebug;
}

// Two separate collision events are being generated per collision between objects.
// As such, we only operate on ourselves, and let other modify itself when it resolves it's own event.
CollisionEvent PhysicsEngine::ResolveCollision(const CollisionData& data)
{
	CollisionEvent event;
	event.other = data.other;
	event.normal = data.collidedNormal;

	Collider* selfColl = data.self->GetCollider();
	Collider* otherColl = data.other->GetCollider();

	Vector2 mtv = data.mtv;

	// If the mtv is pointing towards the other colliding object, turn it in the opposite direction
	const Vector2 directionToOther = data.other->GetPosition() - data.self->GetPosition();
	float direction = Vector2DotProduct(mtv, directionToOther);
	if (direction > 0)
	{
		mtv *= -1;
	}

	event.direction = Vector2Normalize(mtv);
	event.penetrationDepth = data.depth;

	// Only interested in overlaps, and does not affect the other object physically.
	if (selfColl->type == ColliderType::OverlapOnly || otherColl->type == ColliderType::OverlapOnly)
	{
		return event;
	}

	// Static
	if (selfColl->type == ColliderType::Static)
	{
		return event;
	}

	// One movable, one static
	if (selfColl->type == ColliderType::Movable && otherColl->type == ColliderType::Static)
	{
		// We perform the whole movement, since the other collider is static
		data.self->SetPosition(data.self->GetPosition() + mtv);

		return event;
	}

	// Both movable, we split the mtv between us and move ourselves
	mtv *= 0.5f;

	data.self->SetPosition(data.self->GetPosition() + mtv);
	return event;
}

void PhysicsEngine::GetColliderNormals(const Collider& collider, std::vector<Vector2>& normals)
{
	std::vector<Vector2> sides;

	//Calculate the sides of the collider
	for (int i = 0; i < collider.points.size(); i++)
	{
		sides.push_back(collider.points[(i + 1) % collider.points.size()] - collider.points[i]);
	}

	//Calculate the normals of each side
	for (int i = 0; i < sides.size(); i++)
	{
		Vector2 result = GetPerpendicularCCW(Vector2Normalize(sides[i]));
		
		//Filter out similar normals, or normals that are inverted
		for (auto& normal : normals)
		{
			if (Vector2Equals(normal, result) || Vector2Equals(normal, Vector2Multiply(result, { -1, -1 })))
			{
				goto skipPushBack;
			}
		}

		//Draw normals
		normals.push_back(result);

	skipPushBack:
		//To make the goto valid and not throw errors
		int _ = 0;

	}

}

PhysicsEngine::ProjectionRange PhysicsEngine::GetProjection(const Collider& collider, Vector2 normal)
{
	ProjectionRange range;

	bool bFirstMin = true;
	bool bFirstMax = true;

	//Projecting all points on self shape onto current axis
	for (int side = 0; side < collider.points.size(); side++)
	{
		//Dot product 
		float dot = normal.x * (collider.points[side].x) + normal.y * (collider.points[side].y);
		if (dot > range.second || bFirstMax)
		{
			range.second = dot;
			bFirstMax = false;
		}
		if (dot < range.first || bFirstMin)
		{
			range.first = dot;
			bFirstMin = false;
		}
	}

	return range;
}
