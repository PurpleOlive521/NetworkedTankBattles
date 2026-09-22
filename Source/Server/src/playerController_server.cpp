#include "playerController_server.h"
#include "messages.h"
#include "game.h"

ServerPlayerController::ServerPlayerController()
{

}

// We send the client our state and the latest received inputs tick. This is read by ClientPlayerController.
bool ServerPlayerController::Write(meteor::byte_stream_writer& writer)
{
    PlayerState state = PlayerState(controlledTank);
    state.sendTick = latestReceivedTick;

    return state.Write(writer);
}

void ServerPlayerController::Update(float deltaTime)
{
    ActOnInput(latestInput);
}

bool ServerPlayerController::WriteAsParentController(meteor::byte_stream_writer& writer)
{
    return TankController::Write(writer);
}

GameObjectTypes ServerPlayerController::GetParentType() const
{
    return GameObjectTypes::TankController;
}

void ServerPlayerController::ReceiveClientInput(const meteor::input_message& input)
{
    if (latestReceivedTick > input.m_tick)
    {
        debug::warn("Input message dropped - older than latest input received.");
        return;
    }

    latestReceivedTick = input.m_tick;

    InputState inputState;
    for (const auto& [button, isPressed] : input.m_inputs)
    {
        inputState.inputStateMap.insert({ (Button)button, isPressed });
    }

    inputState.mousePosition = Vector2{ (float)input.m_mouse_x, (float)input.m_mouse_y };

    latestInput = inputState;
}
