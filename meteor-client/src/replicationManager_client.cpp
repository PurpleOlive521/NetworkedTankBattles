#include "replicationManager_client.h"
#include "game.h"
#include "eventFactory.h"
#include "bulletController.h"
#include "core.h"
#include "raymath.h"

ClientReplicationManager::ClientReplicationManager(Game* inReplicatedGame) : ReplicationManager(inReplicatedGame)
{
}

void ClientReplicationManager::OnConnectedToServer()
{
}

void ClientReplicationManager::OnDisconnectedFromServer(bool bWasTimeout)
{
}

void ClientReplicationManager::OnReceiveServerMessages(const meteor::payload_packet& payload, byte_stream_reader& reader)
{
	// If no accompanying create message is found for this objects state, the object is removed
	std::vector<ReplicatedObjectPair> temporaryObjects;

	while (reader.has_data())
	{
		message_type messageType = (message_type)reader.peek();
		if (messageType >= message_type::ENUM_MAX || messageType == message_type::NONE)
		{
			debug::warn("Invalid message type!");
			return;
		}

		// Read all replication messages
		switch (messageType)
		{
		case message_type::REPLICATION:
		{
			replication_header message;
			if (!message.read(reader))
			{
				debug::warn("Failed to read replication message!");
				return;
			}

			// Search controllers
			Controller* controller = GetControllerByNetworkId(message.m_networkId);
			if (!controller)
			{
				// Search PlayerControllers
				controller = GetPlayerControllerByNetworkId(message.m_networkId);
				if (!controller)
				{
					// No controller for the id exists. Create a dummy object to serialize into so we can continue reading.
					ReplicatedObjectPair pair = CreateReplicatedObject((GameObjectTypes)message.m_classId, message.m_networkId);
					pair.first->Read(reader);
					temporaryObjects.emplace_back(std::move(pair));

					continue;
				}
			}

			controller->Read(reader);
			controller->OnReplication(payload);

			break;
		}

		case message_type::RELIABLE_MESSAGE:
			messageSystem.ReceiveMessageStream(reader, payload);
			break;

		default:
			debug::warn("Unsupported message type received. Dropping packet.");
			return;

		}

	}


	// We are done reading the stream. Try to find create messages for every temporary object created, otherwise delete them
	// Any non Create/Destroy messages are forwarded to the EventSystem for the game-layer to handle.
	while (ReliableMessagePtr message = messageSystem.FetchLatestMessage())
	{
		if (message->type == (uint8)ReliableMessageType::CreateObject)
		{
			CreateObjectMessage* createMessage = dynamic_cast<CreateObjectMessage*>(message.get());
			if (!createMessage)
			{
				debug::error("CreateMessage was not able to be cast despite its type!");
				return;
			}

			for (size_t i = 0; i < temporaryObjects.size(); i++)
			{
				auto& temporary = temporaryObjects.at(i);
				// The create message is for this object, add it to the game
				if (createMessage->networkId == temporary.first->GetNetworkId())
				{
					PostRegisterObjectPair(std::move(temporary));
					temporaryObjects.erase(temporaryObjects.begin() + i);
					break;
				}
			}
		}

		if (message->type == (uint8)ReliableMessageType::DestroyObject)
		{
			DestroyObjectMessage* destroyMessage = dynamic_cast<DestroyObjectMessage*>(message.get());
			if (!destroyMessage)
			{
				debug::error("DestroyMessage was not able to be cast despite its type!");
				return;
			}

			if (auto it = networkIdToControllerMap.find(destroyMessage->networkId); it != networkIdToControllerMap.end())
			{
				it->second->SetIsPendingKill(true);
			}
	
		}

		// Try and convert it to a event that we can forward to the games EventManager.
		EventBasePtr outEvent = EventFactory::CreateEventFromMessage(message.get(), this);

		if (!outEvent)
		{
			return;
		}

		replicatedGame->GetEventSystem()->QueueEvent(std::move(outEvent));
	}
}

void ClientReplicationManager::Update(float deltaTime)
{
	uint32 tick = replicatedGame->GetGameTick();
	if (tick > lastTick)
	{
		lastTick = tick;

		for (auto& playerController : localPlayerControllers)
		{
			playerController->OnTick();
		}

		for (auto& [networkId, controller] : networkIdToControllerMap)
		{
			controller->OnTick();
		}
	}
}

void ClientReplicationManager::Render() const
{
}

CreatedObjectPair ClientReplicationManager::AddGameObjectFromId(GameObjectTypes types, uint32 inId)
{
	ReplicatedObjectPair pair = CreateReplicatedObject(types, inId);
	CreatedObjectPair outPair = { pair.first.get(), pair.second.get() };

	// Keep track of all PlayerControllers we add.
	if (types == GameObjectTypes::PlayerController)
	{
		localPlayerControllers.push_back(dynamic_cast<ClientPlayerController*>(pair.first.get()));
	}

	RegisterReplicatedObject(std::move(pair));

	return outPair;
}

uint32 ClientReplicationManager::GetNetworkIdOfObject(GameObject* object) const
{
	for (auto& [id, controller] : networkIdToControllerMap)
	{
		if (controller->GetControlledEntity() == object)
		{
			return id;
		}
	}

	for (const auto& playerController : localPlayerControllers)
	{
		if (playerController->GetControlledEntity() == object)
		{
			return playerController->GetNetworkId();
		}
	}

	return NETWORK_ID_NONE;
}

void ClientReplicationManager::OnControllerDestroyed(uint32 networkId)
{
	ReplicationManager::OnControllerDestroyed(networkId);

	auto it = std::find_if(localPlayerControllers.cbegin(), localPlayerControllers.cend(),
		[networkId](const ClientPlayerController* controller)
		{
			return networkId == controller->GetNetworkId();
		}
	);

	if (it != localPlayerControllers.cend())
	{
		localPlayerControllers.erase(it);
	}
	
}

bool ClientReplicationManager::IsPlayerController(uint32 networkId) const
{
	for (const auto& playerController : localPlayerControllers)
	{
		if (playerController->GetNetworkId() == networkId)
		{
			return true;
		}
	}

	return false;
}

ClientPlayerController* ClientReplicationManager::GetPlayerControllerByNetworkId(uint32 networkId)
{
	for (const auto& playerController : localPlayerControllers)
	{
		if (playerController->GetNetworkId() == networkId)
		{
			return playerController;
		}
	}

	return nullptr;
}

ClientPlayerController* ClientReplicationManager::GetPlayerController(int index)
{
	if (index < 0 || index >= localPlayerControllers.size())
	{
		return nullptr;
	}

	return localPlayerControllers[index];
}

void ClientReplicationManager::SendMessage(ReliableMessagePtr message)
{
	messageSystem.SendMessage(message);
}

bool ClientReplicationManager::GetInputWithMessages(const payload_packet& header, byte_stream_writer& writer)
{
	// We send anyways in case the server is about to send us the Create messages necessary for us to
	// replicate our own PlayerController.
	if (ClientPlayerController* playerController = GetPlayerController())
	{
		input_message inputState = playerController->GetInputMessage();
		if (!inputState.write(writer))
		{
			return false;
		}
	}
	else
	{
		debug::warn("No PlayerController found when trying to send input!");
	}

	messageSystem.FillPayloadStream(writer, header);

	return true;
}

void ClientReplicationManager::PostRegisterObjectPair(ReplicatedObjectPair objectPair)
{
	// Keep track of all PlayerControllers we add.
	if (objectPair.first->GetType() == GameObjectTypes::PlayerController)
	{
		localPlayerControllers.push_back(dynamic_cast<ClientPlayerController*>(objectPair.first.get()));
	}

	RegisterReplicatedObject(std::move(objectPair));
}

ReplicatedObjectPair ClientReplicationManager::CreateReplicatedObject(GameObjectTypes type, uint32 networkId) const
{
	ReplicatedObjectPair result;

	switch (type)
	{
	case(GameObjectTypes::Entity):
	case(GameObjectTypes::EntityController):
		result = ReplicatedObjectPair(std::make_unique<EntityController>(), std::make_unique<Entity>());
		break;

	case(GameObjectTypes::PlayerController):
		result = ReplicatedObjectPair(std::make_unique<ClientPlayerController>(), std::make_unique<Tank>());
		break;

	case(GameObjectTypes::TankController):
		result = ReplicatedObjectPair(std::make_unique<TankController>(), std::make_unique<Tank>());
		break;

	case(GameObjectTypes::Bullet):
	case(GameObjectTypes::BulletController):
		result = ReplicatedObjectPair(std::make_unique<BulletController>(), std::make_unique<Bullet>());
		break;

	default:
		assertNoEntry(); // The given type is not supported by the factory or not replicated.
		break;
	}

	if (result.first)
	{
		result.first->SetNetworkId(networkId == NETWORK_ID_NONE ? GenerateNetworkId() : networkId);

		// Otherwise causes a redundant message when a controller is removed by a DeathMessage from the Server.
		result.first->bGenerateDeathMessage = false;
	}

	// We have received a object with a networkId higher than we expected. Try to resolve the issue by matching the id.
	if (nextNetworkid < networkId)
	{
		nextNetworkid = networkId;
	}

	return result;
}