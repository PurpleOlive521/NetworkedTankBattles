#pragma once

#include "eventSystemTypes.h"

class Game;

class BulletManager
{
public:
	BulletManager() = default;

	void Init(Game* game);

	void CreateBullet(EventBase* inEvent);

private:
	Game* owningGame = nullptr;
};