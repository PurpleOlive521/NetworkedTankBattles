#include "inputTypes.h"

InputState::InputState()
{
}

InputState::InputState(const InputStateMap& inState)
{
	inputStateMap = inState;
}

bool InputState::Get(Button button) const
{
	if (auto it = inputStateMap.find(button); it != inputStateMap.end())
	{
		return it->second;
	}

	return false;
}