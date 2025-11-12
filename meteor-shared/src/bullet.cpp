#include "bullet.h"
#include "raymath.h"
#include "utility.h"
#include "tank.h"

void Bullet::BeginPlay()
{
	Entity::BeginPlay();

	bulletTexture = LoadTexture("Player/PlayerBullet.png");

	collider.GenerateRectangle(Vector2{ BULLET_WIDTH, BULLET_HEIGHT }, Vector2{});
	collider.type = ColliderType::OverlapOnly;
}

void Bullet::Update(float deltaTime)
{
	UpdatePhysics(deltaTime);
}

void Bullet::Render()
{
	const Rectangle body = { GetPosition().x, GetPosition().y, BULLET_WIDTH, BULLET_HEIGHT };
	const Rectangle bulletTextureRectangle = {0, 0, BULLET_WIDTH, BULLET_HEIGHT};

	DrawTexturePro(bulletTexture, bulletTextureRectangle, body, GetRectCenter(body), GetRotation(), WHITE);
}

GameObjectTypes Bullet::GetType() const
{
	return GameObjectTypes::Bullet;
}

bool Bullet::IsPhysicsObject() const
{
	return true;
}

Collider* Bullet::GetCollider()
{
	return &collider;
}

void Bullet::OnCollision(Entity* other, const CollisionEvent& event)
{
	if (other == shooter)
	{
		return;
	}

	if (other->GetType() == GameObjectTypes::Tank)
	{
		// Make sure we are hitting at Tank object
		if (Tank* otherTank = dynamic_cast<Tank*>(other))
		{
			OnHitTank(otherTank);
		}

		return;
	}

	if (ricochetsLeft <= 0)
	{
		SetIsPendingKill(true);
	}

	const Vector2 normal = event.normal;
	Vector2 reflection = GetForwardVector() - (normal * 2 * Vector2DotProduct(GetForwardVector(), normal));
	reflection = Vector2Normalize(reflection) * event.penetrationDepth;
	
	const float desiredBulletRotation = ToRayDeg(Atan2Degrees(reflection.y, reflection.x));
	SetRotation(desiredBulletRotation);

	SetPosition(GetPosition() + event.direction * event.penetrationDepth);
	OnRicochet();
}

void Bullet::OnHitTank(Tank* other)
{
	// We do not have authority to kill the other tank.
}

void Bullet::OnRicochet()
{
	// We do not have authority to decrease/kill the bullet.
}

void Bullet::UpdatePhysics(float deltaTime)
{
	// TODO: Make server only action
	const Vector2 pos = GetPosition();
	const bool bIsOutsideScreen = pos.x < 0 || pos.y < 0 || pos.x > GetScreenWidth() || pos.y > GetScreenHeight();
	if (bIsOutsideScreen)
	{
		SetIsPendingKill(true);
	}

	collider.SetVelocity(GetForwardVector() * BULLET_SPEED * deltaTime);
}
