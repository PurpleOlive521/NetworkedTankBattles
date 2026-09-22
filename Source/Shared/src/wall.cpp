#include "wall.h"
#include "raylib.h"
#include "utility.h"

const float width = 100.0f;
const float height = 30.0f;

void Wall::BeginPlay()
{
	Entity::BeginPlay();

	wallTexture = LoadTexture("Wall.png");
	SetTextureWrap(wallTexture, TEXTURE_WRAP_REPEAT);

	collider.GenerateRectangle(Vector2{ width, height }, Vector2{});
}

bool Wall::ShouldUpdate() const
{
	return false;
}

void Wall::Render()
{
	const Vector2 drawPosition = GetPosition();
	const Rectangle rect = { drawPosition.x, drawPosition.y, width, height };
	const Rectangle source = {0, 0, width, height};

	DrawTexturePro(wallTexture, source, rect, GetRectCenter(rect), GetRotation(), WHITE);
	//DrawRectanglePro(rect, GetRectCenter(rect), GetRotation(), BLACK);
}

GameObjectTypes Wall::GetType() const
{
	return GameObjectTypes::Wall;
}

bool Wall::IsPhysicsObject() const
{
	return true;
}

Collider* Wall::GetCollider()
{
	return &collider;
}
