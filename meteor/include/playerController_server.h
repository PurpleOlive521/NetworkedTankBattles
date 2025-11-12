#pragma once

#include "playerController.h"

class ServerPlayerController final : public PlayerController
{
public:
	ServerPlayerController();

	bool Write(meteor::byte_stream_writer& writer) override;

	void Update(float deltaTime) override;

	// Serializes the PlayerController as it's parent class, without any PlayerController-specific data.
	bool WriteAsParentController(meteor::byte_stream_writer& writer);

	// Returns the parents GameObjectType.
	GameObjectTypes GetParentType() const;

	// The clients latest input for this PlayerController
	void ReceiveClientInput(const meteor::input_message& input);

private:

	// Latest input tick we have received and acted upon from the client.
	uint32 latestReceivedTick = 0;

	InputState latestInput = {};
};