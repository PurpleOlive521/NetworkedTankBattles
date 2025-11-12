#pragma once

#include "bullet.h"

class ServerBullet final : public Bullet
{
public:
	ServerBullet() = default;

	void OnHitTank(Tank* other) override;
	void OnRicochet() override;

};