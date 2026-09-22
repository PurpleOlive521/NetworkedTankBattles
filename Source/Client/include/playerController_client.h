#pragma once

#include "playerController.h"
#include "messages.h"

constexpr int MAX_HISTORY_SIZE = 100;

typedef std::pair<uint32, InputState> TickAssociatedInput;

class ClientPlayerController final : public PlayerController, public InputManager::InputListener
{
public:
	ClientPlayerController() = default;

	// --- Object Lifecycle
	void BeginPlay() override;

	void OnDestroy() override;

	void OnTick() override;

	bool Write(meteor::byte_stream_writer& writer) override;

	bool Read(meteor::byte_stream_reader& reader) override;

	// Generates an input_message containing the latest input state for this PlayerController.
	meteor::input_message GetInputMessage() const; 

	// --- Input Handling
	void SetupInput();

	// --- Begin InputManager::InputListener interface
	void ReceiveInputState(const InputState& inputState) override;
	// --- End InputManager::InputListener interface

	void Reconciliate(const PlayerState& receivedState);
		
private:
	InputState latestInput = {};

	std::vector<TickAssociatedInput> inputHistory;
};