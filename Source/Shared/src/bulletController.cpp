#include "bulletController.h"
#include "game.h"
#include "replicationManager.h"

BulletState::BulletState(Bullet* inBullet, BulletController* bulletController)
{
	position = inBullet->GetPosition();
	rotation = inBullet->GetRotation();

	remainingRicochets = inBullet->ricochetsLeft;
	shooterId = bulletController->GetNetworkId();
}

template <typename T>
bool Serialize(BulletState& state, T& stream)
{
	bool success = true;
	success &= stream.serialize(state.position.x);
	success &= stream.serialize(state.position.y);

	success &= stream.serialize(state.rotation);
	success &= stream.serialize(state.shooterId);
	success &= stream.serialize(state.remainingRicochets);

	return success;
}


bool BulletState::Write(meteor::byte_stream_writer& writer)
{
	return Serialize(*this, writer);
}

bool BulletState::Read(meteor::byte_stream_reader& reader)
{
	return Serialize(*this, reader);
}

void BulletState::ApplyOnBullet(Bullet* inBullet, Game* owningGame)
{
	inBullet->ricochetsLeft = remainingRicochets;
	
	ReplicationManager* replicationManager = owningGame->GetReplicationManager();
	inBullet->shooter = replicationManager->GetControllerByNetworkId(shooterId);
	assert(inBullet->shooter); // Controller was not found by ID
}

GameObjectTypes BulletController::GetType() const
{
	return GameObjectTypes::BulletController;
}

void BulletController::OnControlledEntityChanged()
{
	controlledBullet = dynamic_cast<Bullet*>(controlledEntity);
	assert(controlledBullet);
}

bool BulletController::Write(meteor::byte_stream_writer& writer)
{
	BulletState state = BulletState(controlledBullet, this);

	return state.Write(writer);
}

bool BulletController::Read(meteor::byte_stream_reader& reader)
{
	BulletState state;
	if (!state.Read(reader))
	{
		return false;
	}

	// We might be created temporarily to read state in which case we wont have any game or controlled objects.
	if (controlledBullet)
	{
		state.ApplyOnBullet(controlledBullet, owningGame);
	}

	RecordState(GetTime(), state.position, state.rotation);

	return true;
}