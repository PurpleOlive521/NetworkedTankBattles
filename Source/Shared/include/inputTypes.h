#pragma once

#include "core.h"
#include <map>
#include "raylib.h"

enum class Button : uint8
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	MOUSE_LEFT,
};

typedef std::map<Button, bool> InputStateMap;

struct InputState
{
	InputState();

	explicit InputState(const InputStateMap& inState);

	// Returns the state of the requested button. Returns false if Button was not found.
	bool Get(Button button) const;

	InputStateMap inputStateMap;

	Vector2 mousePosition = {};
};