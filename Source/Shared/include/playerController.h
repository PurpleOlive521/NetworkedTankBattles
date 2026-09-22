#pragma once

#include "inputManager.h"
#include "tankController.h"

struct PlayerState
{
    PlayerState() = default;
    // Initializes the TankState. Still requires additional information to be inserted.
    PlayerState(Tank* inTank);

    bool Write(meteor::byte_stream_writer& writer);
    bool Read(meteor::byte_stream_reader& reader);

    uint32 sendTick = 0U; // The tick that the latest received input was sent at.
    TankState tankState;
};

class PlayerController : public TankController
{
public:
	PlayerController();

	virtual GameObjectTypes GetType() const override;

	void ActOnInput(const InputState& inputState);
};