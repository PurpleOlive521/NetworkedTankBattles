#include "playerController_client.h"
#include "Game.h"
#include "raymath.h"

void ClientPlayerController::BeginPlay()
{
	PlayerController::BeginPlay();

	controlledTank->tankBaseTexture = LoadTexture("Player/PlayerBase.png");
	controlledTank->tankTurretTexture = LoadTexture("Player/PlayerTurret.png");

	SetupInput();
}

void ClientPlayerController::OnDestroy()
{
	owningGame->GetInputManager()->UnregisterListener(this);

	PlayerController::OnDestroy();
}

void ClientPlayerController::OnTick()
{
	PlayerController::OnTick();

	inputHistory.push_back(TickAssociatedInput(owningGame->GetGameTick(), latestInput));
}

bool ClientPlayerController::Write(meteor::byte_stream_writer& writer)
{
	return false;
}

// We expect the server to send us our state at the latest tick. We will then reconcile on this.
bool ClientPlayerController::Read(meteor::byte_stream_reader& reader)
{
	PlayerState state;
	if (!state.Read(reader))
	{
		debug::warn("Could not read player state!");
		return false;
	}

	Reconciliate(state);

	return true;
}

meteor::input_message ClientPlayerController::GetInputMessage() const
{
	meteor::input_message message = { owningGame->GetGameTick(), latestInput };
	return message;
}

void ClientPlayerController::SetupInput()
{
	owningGame->GetInputManager()->RegisterListener(this);
}

void ClientPlayerController::ReceiveInputState(const InputState& inputState)
{
	latestInput = inputState;
	ActOnInput(inputState);
}

void ClientPlayerController::Reconciliate(const PlayerState& receivedState)
{
	// If we were just created, or temporarily created to read state into, we wont have a tank to act on. 
	if (!controlledTank)
	{
		return;
	}

	receivedState.tankState.ApplyToTank(controlledTank);


	// Remove all elements that are older than the received & validated input.
	for(int i = (int)inputHistory.size() - 1; i > -1; i--)
	{
		if (inputHistory[i].first <= receivedState.sendTick)
		{
			inputHistory.erase(inputHistory.begin() + i);
		}
	}

	uint32 lastAppliedTick = 0U;

	// Act on all remaining input, simulating physics updates based on the time between ticks.
	for (const auto& [tick, inputState] : inputHistory)
	{
		uint32 tickDelta = tick - lastAppliedTick;

		if (lastAppliedTick == 0)
		{
			tickDelta = 1;
		}

		double simulatedDeltatime = tickDelta * owningGame->GetGameTickLength();
		ActOnInput(inputState);
		controlledTank->UpdatePhysics((float)simulatedDeltatime);

		lastAppliedTick = tick;
	}
}
