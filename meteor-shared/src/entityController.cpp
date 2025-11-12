#include "entityController.h"
#include "game.h"
#include "utility.h"

InterpolationState::InterpolationState(double inReceiveTime, Vector2 inPosition, float inRotation)
{
	receivedTime = inReceiveTime;
	position = inPosition;
	rotation = inRotation;
}

void EntityController::BeginPlay()
{
	Controller::BeginPlay();
}

void EntityController::Update(float DeltaTime)
{
	Controller::Update(DeltaTime);
	
	if (!bShouldInterpolate)
	{
		return;
	}

	// We don't have enough data to interpolate, so we try to use the latest state as a fallback
	if (stateHistory.size() < 2)
	{
		if (stateHistory.size() > 0)
		{
			const auto& latestState = stateHistory[stateHistory.size() - 1];
			controlledEntity->SetPosition(latestState.position);
			controlledEntity->SetRotation(latestState.rotation);
		}
		return;
	}

	const double replicationTime = GetTime();
	
	const int lastIndex = (int)stateHistory.size() - 1;
	const auto& stateFrom = stateHistory[lastIndex - 1]; // Point that we are interpolating from
	const auto& stateTarget = stateHistory[lastIndex]; // Current target that we are interpolating to

	double alpha = (replicationTime - stateTarget.receivedTime) / (stateTarget.receivedTime - stateFrom.receivedTime);

	Vector2 finalPosition = Vector2Lerp(stateFrom.position, stateTarget.position, (float)alpha);
	float finalRotation = LerpRotation(stateFrom.rotation, stateTarget.rotation, (float)alpha);

	controlledEntity->SetPosition(finalPosition);
	controlledEntity->SetRotation(finalRotation);
}

void EntityController::OnDestroy()
{
	Controller::OnDestroy();
}

void EntityController::OnReplication(const meteor::payload_packet& payload)
{
	if (!bShouldInterpolate)
	{
		return;
	}

	if (!controlledEntity)
	{
		return;
	}

	if (stateHistory.size() >= 4)
	{
		stateHistory.erase(stateHistory.begin());
	}
}

bool EntityController::Write(meteor::byte_stream_writer& writer)
{
	bool bSuccess = true;

	Vector2 position = controlledEntity->GetPosition();
	float rotation = controlledEntity->GetRotation();
	bSuccess &= writer.serialize(position.x);
	bSuccess &= writer.serialize(position.y);
	bSuccess &= writer.serialize(rotation);

	return bSuccess;
}

bool EntityController::Read(meteor::byte_stream_reader& reader)
{
	bool bSuccess = true;

	Vector2 position = {};
	float rotation = {};
	bSuccess &= reader.serialize(position.x);
	bSuccess &= reader.serialize(position.y);
	bSuccess &= reader.serialize(rotation);

	const double time = GetTime();
	RecordState(time, position, rotation);

	return bSuccess;
}

GameObjectTypes EntityController::GetType() const
{
	return GameObjectTypes::EntityController;
}

void EntityController::RecordState(double receiveTime, Vector2 position, float rotation)
{	
	stateHistory.push_back(InterpolationState(receiveTime, position, rotation));
}

