#include "playerController.h"
#include "game.h"

template <typename T>
bool Serialize(PlayerState& state, T& stream)
{
	bool success = true;
	success &= stream.serialize(state.sendTick);

	return success;
}

PlayerState::PlayerState(Tank* inTank) : tankState(inTank)
{
}

bool PlayerState::Write(meteor::byte_stream_writer& writer)
{
	if (!tankState.Write(writer))
	{
		return false;
	}

	return Serialize(*this, writer);
}

bool PlayerState::Read(meteor::byte_stream_reader& reader)
{
	if (!tankState.Read(reader))
	{
		return false;
	}

	return Serialize(*this, reader);
}

PlayerController::PlayerController()
{
	bShouldInterpolate = false;
}

GameObjectTypes PlayerController::GetType() const
{
	return GameObjectTypes::PlayerController;
}

void PlayerController::ActOnInput(const InputState& inputState)
{
	if (inputState.Get(Button::UP))
	{
		controlledTank->Accelerate();
	}

	if (inputState.Get(Button::DOWN))
	{
		controlledTank->Reverse();
	}

	if (inputState.Get(Button::LEFT))
	{
		controlledTank->TurnLeft();
	}

	if (inputState.Get(Button::RIGHT))
	{
		controlledTank->TurnRight();
	}

	controlledTank->Aim(inputState.mousePosition);

	if (inputState.Get(Button::MOUSE_LEFT))
	{
		controlledTank->TryShoot();
	}
}
