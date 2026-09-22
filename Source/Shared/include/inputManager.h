#pragma once

#include <vector>
#include "inputTypes.h"

class Game;

constexpr int INVALID_LISTENER = -1;

// Update all registered controllers.
class InputManager
{
public:

	class InputListener
	{
	public:
		virtual void ReceiveInputState(const InputState& inputState) = 0;
	};

	InputManager(Game* inOwningGame);

	void Update(float deltaTime);

	void RegisterListener(InputListener* controller);
	void UnregisterListener(InputListener* controller);

	// Returns nullptr if none exists at the given index.
	InputListener* GetListener(int index);

protected:
	// Returns INVALID_LISTENER if none is found.
	int GetListenerIndex(InputListener* controller);

	std::vector<InputListener*> listeners;

	InputStateMap buttonStates;

	Game* owningGame;
};