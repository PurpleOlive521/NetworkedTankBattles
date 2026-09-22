#pragma once

#include "entity.h"
#include "physicsTypes.h"

class Wall final: public Entity
{
public:
	Wall() = default;

	void BeginPlay() override;

	bool ShouldUpdate() const override;

	void Render() override;

	GameObjectTypes GetType() const override;

	bool IsPhysicsObject() const override;

	Collider* GetCollider() override;

private:

	Collider collider;
	
	Texture2D wallTexture; 

};
