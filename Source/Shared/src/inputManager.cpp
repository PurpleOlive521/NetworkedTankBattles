#include "inputManager.h"
#include "game.h"
#include "raylib.h"

InputManager::InputManager(Game* inOwningGame)
{
	assert(inOwningGame);
	owningGame = inOwningGame;

	buttonStates[Button::UP] = false;
	buttonStates[Button::DOWN] = false;
	buttonStates[Button::LEFT] = false;
	buttonStates[Button::RIGHT] = false;
	buttonStates[Button::MOUSE_LEFT] = false;
}

void InputManager::Update(float deltaTime)
{
	for (auto& key : buttonStates)
	{
		switch (key.first)
		{
		case(Button::DOWN):
			key.second = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
			break;
		case(Button::UP):
			key.second = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
			break;
		case(Button::LEFT):
			key.second = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
			break;
		case(Button::RIGHT):
			key.second = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
			break;
		case(Button::MOUSE_LEFT):
			key.second = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
			break;
		}
	}

	InputState outState = InputState(buttonStates);
	outState.mousePosition = GetMousePosition();

	for (auto& inputListener : listeners)
	{
		inputListener->ReceiveInputState(outState);
	}
}

void InputManager::RegisterListener(InputListener* listener)
{
	listeners.push_back(listener);
}

void InputManager::UnregisterListener(InputListener* listener)
{
	if (GetListenerIndex(listener) != INVALID_LISTENER)
	{
		listeners.erase(listeners.begin() + GetListenerIndex(listener));
	}
}

InputManager::InputListener* InputManager::GetListener(int index)
{
	if(index >= 0 && index < (int)listeners.size())
	{
		return listeners[index];
	}

	return nullptr;
}

int InputManager::GetListenerIndex(InputListener* controller)
{
	for(size_t i = 0; i < listeners.size(); i++)
	{
		if(listeners[i] == controller)
		{
			return (int)i;
		}
	}

	return INVALID_LISTENER;
}
