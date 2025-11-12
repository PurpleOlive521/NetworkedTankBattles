#pragma once

#include "level.h"

class Level_Basic : public Level
{
public:
	Level_Basic();

	void InstantiateLevel(Game* game) override;
};