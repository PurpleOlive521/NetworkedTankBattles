#include "entity.h"
#include "Game.h"
#include "raymath.h"
#include "utility.h"

void Entity::Update(float deltaTime)
{

}

void Entity::Render()
{
	DrawCircle(int(position.x), int(position.y), 10.f, PINK);
}

GameObjectTypes Entity::GetType() const
{
	return GameObjectTypes::Entity;
}

void Entity::OnDestroy()
{
	if (controller)
	{
		controller->SetIsPendingKill(true);
		controller = nullptr;
	}
}

bool Entity::IsPhysicsObject() const
{
	return false;
}

Collider* Entity::GetCollider()
{
	return nullptr;
}

void Entity::OnCollision(Entity* other, const CollisionEvent& event)
{
}

void Entity::SetPosition(Vector2 inPosition)
{
	position = inPosition;
}

Vector2 Entity::GetPosition() const
{
	return position;
}

void Entity::SetRotation(float inRotation)
{
	rotation = inRotation;

	if (rotation > 360.0f)
	{
		rotation -= 360.0f;
	}
	else if(rotation < -360.0f)
	{
		rotation += 360.0f;
	}

	forwardVector = Vector2{ sinf(DEG2RAD * rotation), -cosf(DEG2RAD * rotation) };
}

float Entity::GetRotation() const
{
	return rotation;
}

Vector2 Entity::GetForwardVector() const
{
	return forwardVector;
}
