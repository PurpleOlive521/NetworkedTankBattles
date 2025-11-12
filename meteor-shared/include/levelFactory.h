#pragma once

#include "level.h"

class LevelFactory
{
private:	
	LevelFactory() = default;

public:

	static Level* CreateLevel(LevelId inId);
}; 