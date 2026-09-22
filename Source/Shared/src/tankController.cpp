#include "tankController.h"

TankState::TankState(Tank* inTank)
{
	position = inTank->GetPosition();
	rotation = inTank->GetRotation();
	turretRotation = inTank->turretRotation;
	aimTarget = inTank->aimTarget;
	bIsDead = inTank->bIsDead;
}

template <typename T>
bool Serialize(TankState& state, T& stream)
{
    bool success = true;
    success &= stream.serialize(state.position.x);
    success &= stream.serialize(state.position.y);

    success &= stream.serialize(state.aimTarget.x);
    success &= stream.serialize(state.aimTarget.y);

    success &= stream.serialize(state.rotation);
    success &= stream.serialize(state.turretRotation);
    success &= stream.serialize(state.bIsDead);

    return success;
}

bool TankState::Write(meteor::byte_stream_writer& writer)
{
	return Serialize(*this, writer);
}

bool TankState::Read(meteor::byte_stream_reader& reader)
{
	return Serialize(*this, reader);
}

void TankState::ApplyOnInterpolatedTank(Tank* inTank) const
{
	// Position & rotation is interpolated separately
	inTank->turretRotation = turretRotation;
	inTank->aimTarget = aimTarget;
	inTank->bIsDead = bIsDead;
}

void TankState::ApplyToTank(Tank* inTank) const
{
	inTank->SetPosition(position);
	inTank->SetRotation(rotation);
	inTank->turretRotation = turretRotation;
	inTank->aimTarget = aimTarget;
	inTank->bIsDead = bIsDead;
}



GameObjectTypes TankController::GetType() const
{
	return GameObjectTypes::TankController;
}

void TankController::OnControlledEntityChanged()
{
	controlledTank = dynamic_cast<Tank*>(controlledEntity);
	assert(controlledTank);
}

bool TankController::Write(meteor::byte_stream_writer& writer)
{
	TankState state = TankState(controlledTank);

	return state.Write(writer);
}

bool TankController::Read(meteor::byte_stream_reader& reader)
{
	TankState state;
	if (!state.Read(reader))
	{
		return false;
	}

	// We might be created temporarily to read state in which case we wont have any game or controlled objects.
	if (controlledTank)
	{
		state.ApplyOnInterpolatedTank(controlledTank);
	}

	const double time = GetTime();
	RecordState(time, state.position, state.rotation);

	return true;
}

