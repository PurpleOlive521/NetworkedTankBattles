#include "replicationManager.h"

#include "application.h"
#include "messages.h"
#include "protocol.h"
#include "entity.h"
#include "game.h"

uint32 ReplicationManager::nextNetworkid = 0;

ReplicationManager::ReplicationManager(Game* inReplicatedGame)
{
	assert(inReplicatedGame);
	replicatedGame = inReplicatedGame;
}

void ReplicationManager::Update(float deltaTime)
{
}

void ReplicationManager::Render() const
{

}

void ReplicationManager::OnControllerDestroyed(uint32 networkId)
{
	auto it = networkIdToControllerMap.find(networkId);

	if (it != networkIdToControllerMap.end())
	{
		networkIdToControllerMap.erase(it);
	}
}

Controller* ReplicationManager::GetControllerByNetworkId(uint32 networkId) const
{
	if (networkIdToControllerMap.contains(networkId))
	{
		return networkIdToControllerMap.at(networkId);
	}

	return nullptr;
}

void ReplicationManager::RegisterReplicatedObject(ReplicatedObjectPair pair)
{
	GameObject* controller = nullptr;
	// GameObject first in case the Controller depends on it for it's BeginPlay
	if (pair.second)
	{
		controller = replicatedGame->AddGameObject(std::move(pair.second));
	}

	if (pair.first)
	{
		// Controllers are always created with a GameObject, so this should never be null.
		pair.first->SetReplicationManager(this);

		pair.first->SetControlledEntity(static_cast<Entity*>(controller));

		// We are not responsible for storing PlayerControllers
		if (pair.first->GetType() != GameObjectTypes::PlayerController)
		{
			networkIdToControllerMap[pair.first->GetNetworkId()] = pair.first.get();
		}

		replicatedGame->AddGameObject(std::move(pair.first));
	}
}

uint32 ReplicationManager::GenerateNetworkId()
{
	const uint32 outId = nextNetworkid;

	nextNetworkid++;
	if (nextNetworkid >= UINT32_MAX)
	{
		assertNoEntry() // Hold it there, cowboy! How many controllers do you think you are creating?
	}

	return outId;
}

