#pragma once

#include "network.h"
#include <unordered_map>
#include "controller.h"
#include "reliableMessagingSystem.h"
#include "reliableMessageFactory.h"

using namespace meteor;

class Game;

typedef std::pair<std::unique_ptr<Controller>, std::unique_ptr<GameObject>> ReplicatedObjectPair; // Replicated objects with ownership.
typedef std::pair<Controller*, GameObject*> CreatedObjectPair; // Created objects without ownership.

constexpr uint32 NETWORK_ID_NONE = 0;

class ReplicationManager
{
public:
	ReplicationManager(Game* inReplicatedGame);

	// Sends out OnTick calls.
	virtual void Update(float deltaTime);

	virtual void Render() const;

	// Creates both the object and the corresponding Controller. Generates a new networkId if NETWORK_ID_NONE is passed.
	virtual CreatedObjectPair AddGameObjectFromId(GameObjectTypes types, uint32 inId = NETWORK_ID_NONE) = 0;

	// Queues up the message to be sent.
	virtual void SendMessage(ReliableMessagePtr message) = 0;

	virtual uint32 GetNetworkIdOfObject(GameObject* object) const = 0;

	// Called by the Controllers themselves at OnDestroy.
	virtual void OnControllerDestroyed(uint32 networkId);

	// Returns nullptr if no controller with the given networkId exists. Does not search PlayerControllers.
	Controller* GetControllerByNetworkId(uint32 networkId) const;

protected:

	// Creates the Controller and GameObject for the given type. Is replicated.
	// Pass NETWORK_ID_NONE for creating new objects, or the desired networkId for objects that are being replicated locally!
	virtual ReplicatedObjectPair CreateReplicatedObject(GameObjectTypes	type, uint32 networkId = NETWORK_ID_NONE) const = 0;

	void RegisterReplicatedObject(ReplicatedObjectPair pair);

	// Generates a new network id to use.
	static uint32 GenerateNetworkId();

	static uint32 nextNetworkid;

	// Does not include PlayerControllers, they need to be stored & looked up separately in derived classes!
	std::unordered_map<uint32, Controller*> networkIdToControllerMap;

	uint32 lastTick = 0U;

	Game* replicatedGame;

};