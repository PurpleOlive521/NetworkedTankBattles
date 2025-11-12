#pragma once

#include "replicationManager.h"
#include "playerController_client.h"
#include "Client.h"

using namespace meteor;

class ClientReplicationManager final : public ReplicationManager, public meteor::Client::Listener
{
public:
	ClientReplicationManager(Game* inReplicatedGame);

	// --- Begin Client::Listener interface
	void OnConnectedToServer() override;

	void OnDisconnectedFromServer(bool bWasTimeout) override;

	void OnReceiveServerMessages(const meteor::payload_packet& payload, byte_stream_reader& reader) override;
	// --- End Client::Listener interface

	void Update(float deltaTime) override;

	void Render() const override;

	// Creates the Controller and GameObject for the given type. Is replicated.
	CreatedObjectPair AddGameObjectFromId(GameObjectTypes types, uint32 inId = NETWORK_ID_NONE) override;

	// Returns NETWORK_ID_NONE if not found. Searches both PlayerControllers and regular controllers.
	uint32 GetNetworkIdOfObject(GameObject* object) const override;

	void OnControllerDestroyed(uint32 networkId) override;

	// Returns true if the controller is a local PlayerController.
	bool IsPlayerController(uint32 networkId) const;

	// Returns nullptr if no controller is associated with the networkId.
	ClientPlayerController* GetPlayerControllerByNetworkId(uint32 networkId);

	// Returns nullptr if a controlled could not be found at the given index.
	ClientPlayerController* GetPlayerController(int index = 0);

	// Queues up the message to be sent.
	void SendMessage(ReliableMessagePtr message) override;

	bool GetInputWithMessages(const payload_packet& header, byte_stream_writer& writer);

protected:

	// Registers the pair. Use on objects that were created through CreateReplicatedObject instead of AddGameObjectFromId.
	void PostRegisterObjectPair(ReplicatedObjectPair objectPair);

	// Objects created through this still needs to be registered to be added to the game lifecycle.
	ReplicatedObjectPair CreateReplicatedObject(GameObjectTypes	type, uint32 networkId = NETWORK_ID_NONE) const override;

private:

	std::vector<ClientPlayerController*> localPlayerControllers;

	ReliableMessagingSystem messageSystem;
};