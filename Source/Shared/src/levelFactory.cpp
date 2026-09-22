#include "levelFactory.h"
#include "level_basic.h"

Level* LevelFactory::CreateLevel(LevelId inId)
{
	switch (inId)
	{
	case LevelId::Basic:
		return new Level_Basic();
		break;

	default:
		assertNoEntry(); // Id not supported yet.
	}

	return nullptr;
}
