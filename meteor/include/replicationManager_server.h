#pragma once

#include "replicationManager.h"
#include "messages.h"
#include "Server.h"
#include "playerController_server.h"
#include "eventSystem.h"

using namespace meteor;

class ServerReplicationManager final : public ReplicationManager, public meteor::Server::Listener, public EventSystem::EventListener
{
public:
	ServerReplicationManager(Game* inReplicatedGame);

	void Init();

	// --- Begin Server::Listener interface
	uint32 OnClientConnected(uint32 clientId) override;

	void OnClientDisconnected(uint32 clientId, bool bWasTimeout) override;

	void OnReceiveClientMessages(const meteor::payload_packet& payload, byte_stream_reader& reader, uint32 clientId) override;
	// --- End Server::Listener interface

	// --- Begin EventListener interface
	void OnEventQueued(EventBase* event) override;
	// --- End EventListener interface

	void Update(float deltaTime) override;

	void Render() const override;

	// Creates the Controller and GameObject for the given type. Is replicated.
	CreatedObjectPair AddGameObjectFromId(GameObjectTypes types, uint32 inId = NETWORK_ID_NONE) override;

	// Returns NETWORK_ID_NONE if not found. Searches both PlayerControllers and regular controllers.
	[[nodiscard("Expensive!")]] uint32 GetNetworkIdOfObject(GameObject* object) const override;

	void OnControllerDestroyed(uint32 networkId) override;

	// Creates the Controller & GameObject and adds it to the repliated game. Always returns the GameObject, not the controller.
	GameObject* CreateAndAddObject(GameObjectTypes inType);

	// Writes the game state to a specific client. Returns true if successful.
	bool GetGameStateWithMessages(uint32 clientId, const payload_packet& header, byte_stream_writer& writer);

	// Returns true if the controller is a registered PlayerController.
	bool IsPlayerController(uint32 networkId) const;

	// Queues up the message to be sent.
	void SendMessage(ReliableMessagePtr message) override;

	// Fills outMessages with any messages received. Messages from any number of clients may be present, but they are in order within outMessages.
	void FetchReceivedMessages(std::vector<ReliableMessagePtr>& outMessages);

	// Clears the received messages from all clients.
	void ClearAllReceivedMessages();

	// Adds the PlayerController and other supporting structure for a new connecting player.
	uint32 AddConnectingPlayer(uint32 clientId);

private:
	ReplicatedObjectPair CreateReplicatedObject(GameObjectTypes	type, uint32 networkId = NETWORK_ID_NONE) const override;

	// Removes the Create message for this object from our crucial messages, if it exists.
	void RemoveMatchingCreateMessage(DestroyObjectMessage* message);

	void ReceiveInputMessage(const meteor::payload_packet& payload, byte_stream_reader& reader, uint32 clientId);

	void ReceiveReliableMessages(const meteor::payload_packet& payload, byte_stream_reader& reader, uint32 clientId);

	// Maps clientId to their PlayerController.
	std::unordered_map<uint32, ServerPlayerController*> clientToPlayerController;

	// Maps clientId to it's own ReliableMessagingSystem.
	std::unordered_map<uint32, ReliableMessagingSystem> clientToMessagingSystem;

	// Messages that are crucial for other connecting clients to receive, even when connecting after the message has been created.
	std::vector<ReliableMessagePtr> criticalMessages;

};