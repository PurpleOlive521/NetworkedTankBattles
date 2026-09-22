#include "physicsTypes.h"
#include "utility.h"

void Collider::SetInitialPoints(std::vector<Vector2>& inPoints)
{
	points = inPoints;
}

void Collider::GenerateRectangle(Vector2 size, Vector2 center)
{
	points.clear();

	const float halfWidth = size.x / 2.0f;
	const float halfHeight = size.y / 2.0f;

	const Vector2 upLeft =		{ center.x - halfWidth, center.y - halfHeight };
	const Vector2 upRight =		{ center.x + halfWidth, center.y - halfHeight };
	const Vector2 downRight =	{ center.x + halfWidth, center.y + halfHeight };
	const Vector2 downLeft =	{ center.x - halfWidth, center.y + halfHeight };

	points.emplace_back(upLeft);
	points.emplace_back(upRight);
	points.emplace_back(downRight);
	points.emplace_back(downLeft);
}

Collider Collider::Project(Vector2 position, float rotation) const
{
	Collider outCollider;

	for (auto point : points)
	{
		point = RotateDeg(point, rotation);
		point += position;
		outCollider.points.emplace_back(point);
	}

	return outCollider;
}

Vector2 Collider::GetVelocity() const
{
	return velocity;
}

void Collider::SetVelocity(Vector2 inVelocity)
{
	velocity = inVelocity;
}
