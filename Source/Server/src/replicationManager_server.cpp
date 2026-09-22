#include "replicationManager_server.h"

#include "reliableMessageFactory.h"
#include "Game.h"
#include "tank_server.h"
#include "bulletController.h"
#include "bullet_server.h"
#include "raymath.h"

ServerReplicationManager::ServerReplicationManager(Game* inReplicatedGame) : ReplicationManager(inReplicatedGame)
{

}

void ServerReplicationManager::Init()
{
    replicatedGame->GetEventSystem()->AddListener(this);
}

uint32 ServerReplicationManager::OnClientConnected(uint32 clientId)
{
    // A controller already exists for the client, return it
    if (clientToPlayerController.find(clientId) != clientToPlayerController.end())
    {
        return clientToPlayerController[clientId]->GetNetworkId();
    }

    return AddConnectingPlayer(clientId);
}

void ServerReplicationManager::OnClientDisconnected(uint32 clientId, bool bWasTimeout)
{
    if (clientToPlayerController.find(clientId) == clientToPlayerController.end())
    {
        debug::warn("Clients PlayerController could not be found when disconnecting!");
        return;
    }

    Controller* controller = clientToPlayerController[clientId];

    if (controller)
    {
        controller->SetIsPendingKill(true);
    }

    clientToPlayerController.erase(clientId);
    clientToMessagingSystem.erase(clientId);
}

void ServerReplicationManager::OnReceiveClientMessages(const meteor::payload_packet& payload, byte_stream_reader& reader, uint32 clientId)
{
    while (reader.has_data())
    {
        message_type messageType = (message_type)reader.peek();
        if (messageType >= message_type::ENUM_MAX || messageType == message_type::NONE)
        {
            debug::warn("Invalid message type!");
            return;
        }

        switch (messageType)
        {
        case message_type::INPUT:
            ReceiveInputMessage(payload, reader, clientId);
            break;

        case message_type::RELIABLE_MESSAGE:
            ReceiveReliableMessages(payload, reader, clientId);
            return;

        default:
            debug::warn("Unrecognised message received by client!");
            return;
        }
     
    }

}


void ServerReplicationManager::OnEventQueued(EventBase* event)
{
    // Try and convert it to a message.
    ReliableMessagePtr outMessage = ReliableMessageFactory::CreateMessageFromEvent(event, this);

    if (!outMessage)
    {
        return;
    }

    SendMessage(outMessage);
}

void ServerReplicationManager::Update(float deltaTime)
{
    uint32 tick = replicatedGame->GetGameTick();
    if (tick > lastTick)
    {
        lastTick = tick;

        for (auto& [networkId, playerController] : clientToPlayerController)
        {
            playerController->OnTick();
        }

        for (auto& [networkId, controller] : networkIdToControllerMap)
        {
            controller->OnTick();
        }
    }

    // We are currently not expecting any messages from the client, so we clear them out every frame to avoid 
    // them piling up while waiting for a fetch request that will never come.
    ClearAllReceivedMessages();
}

void ServerReplicationManager::Render() const
{
    std::string drawString;

    for (const auto& [clientId, messageSystem] : clientToMessagingSystem)
    {
        drawString += messageSystem.GetDebugString() + ENDLINE;
    }

    const Vector2 drawPosTitle = { GetScreenWidth() - 505.0f, GetScreenHeight() - 230.0f };
    const Vector2 drawPosItem = drawPosTitle + Vector2{ 30.0f, 30.0f };

    DrawText("Ack History (O for lost packet, X for received)", (int)drawPosTitle.x, (int)drawPosTitle.y, 20, RAYWHITE);
    DrawText(drawString.c_str(), (int)drawPosItem.x, (int)drawPosItem.y, 20, RAYWHITE);
}

CreatedObjectPair ServerReplicationManager::AddGameObjectFromId(GameObjectTypes types, uint32 inId)
{
    ReplicatedObjectPair pair = CreateReplicatedObject(types, inId);

    CreatedObjectPair outPair = { pair.first.get(), pair.second.get() };
    RegisterReplicatedObject(std::move(pair));

    // Send a message about this new object
    ReliableMessagePtr message = ReliableMessageFactory::CreateMessage(ReliableMessageType::CreateObject);
    CreateObjectMessage* createdMessage = static_cast<CreateObjectMessage*>(message.get());
    createdMessage->classId = (uint8)types;
    createdMessage->networkId = outPair.first->GetNetworkId();
    SendMessage(message);

    return outPair;
}

uint32 ServerReplicationManager::GetNetworkIdOfObject(GameObject* object) const
{
    for (auto& [id, controller] : networkIdToControllerMap)
    {
        if (controller->GetControlledEntity() == object)
        {
            return id;
        }
    }

    for (auto& [clientId, playerController] : clientToPlayerController)
    {
        if (playerController->GetControlledEntity() == object)
        {
            return playerController->GetNetworkId();
        }
    }

    return NETWORK_ID_NONE;
}

void ServerReplicationManager::OnControllerDestroyed(uint32 networkId)
{
    ReplicationManager::OnControllerDestroyed(networkId);

    auto it = std::find_if(clientToPlayerController.cbegin(), clientToPlayerController.cend(),
        [networkId](const std::pair<uint32, ServerPlayerController*>& pair)
        {
            return networkId == pair.second->GetNetworkId();
        }
    );

    if (it != clientToPlayerController.cend())
    {
        clientToPlayerController.erase(it);
    }
}

GameObject* ServerReplicationManager::CreateAndAddObject(GameObjectTypes inType)
{
    CreatedObjectPair pair = AddGameObjectFromId(inType);

    return pair.second;
}

bool ServerReplicationManager::GetGameStateWithMessages(uint32 inClientId, const payload_packet& header, byte_stream_writer& writer)
{
    // Write all PlayerControllers into the stream.
    for (auto& [clientId, playerController] : clientToPlayerController)
    {
        // Clients own PlayerController is replicated normally.
        if (clientId == inClientId)
        {
            replication_header messageHeader(playerController->GetNetworkId(), (uint8)playerController->GetType());
            if (!messageHeader.write(writer))
            {
                return false;
            }

            if (!playerController->Write(writer))
            {
                return false;
            }

            continue;
        }

        // Any other client is replicated as a parent non-PlayerController type, since the clients do not need the PlayerController to be replicated.
        replication_header messageHeader(playerController->GetNetworkId(), (uint8)playerController->GetParentType());
        if (!messageHeader.write(writer))
        {
            return false;
        }

        if (!playerController->WriteAsParentController(writer))
        {
            return false;
        }
    }

    // Write all objects state into the stream.
    for (auto& [id, controller] : networkIdToControllerMap)
    {
        replication_header messageHeader(controller->GetNetworkId(), (uint8)controller->GetType());
        if (!messageHeader.write(writer))
        {
            return false;
        }

        if (!controller->Write(writer))
        {
            return false;
        }
    }

    // Fill any remaining space with messages
    if (auto it = clientToMessagingSystem.find(inClientId); it != clientToMessagingSystem.end())
    {
        it->second.FillPayloadStream(writer, header);
    }
    else
    {
        debug::warn("No MessagingSystem was found for the client! Could not fill payload stream with messages.");
    }

    return true;
}

bool ServerReplicationManager::IsPlayerController(uint32 networkId) const
{
    for (const auto& [clientId, playerController] : clientToPlayerController)
    {
        if (playerController->GetNetworkId() == networkId)
        {
            return true;
        }
    }

    return false;
}

void ServerReplicationManager::SendMessage(ReliableMessagePtr message)
{
    if (message->bIsCritical)
    {
        criticalMessages.push_back(message);
    }

    // When we received Destroy messages we should remove the Create messages too
    if ((ReliableMessageType)message->type == ReliableMessageType::DestroyObject)
    {
        DestroyObjectMessage* destroyMessage = static_cast<DestroyObjectMessage*>(message.get());
        RemoveMatchingCreateMessage(destroyMessage);
    }

    // Send the message forward to all clients
    for(auto& [clientId, messageSystem] : clientToMessagingSystem)
    {
        messageSystem.SendMessage(message);
    }
}

void ServerReplicationManager::FetchReceivedMessages(std::vector<ReliableMessagePtr>& outMessages)
{
    for (auto& [clientId, messageSystem] : clientToMessagingSystem)
    {
        while (ReliableMessagePtr receivedMessage = messageSystem.FetchLatestMessage())
        {
            outMessages.push_back(receivedMessage);
        }
    }
}

void ServerReplicationManager::ClearAllReceivedMessages()
{
    for (auto& [clientId, messageSystem] : clientToMessagingSystem)
    {
        messageSystem.ClearReceivedMessages();
    }
}

uint32 ServerReplicationManager::AddConnectingPlayer(uint32 clientId)
{
    clientToMessagingSystem.insert({ clientId, ReliableMessagingSystem() });
    clientToMessagingSystem[clientId].SendMessages(criticalMessages);

    CreatedObjectPair pair = AddGameObjectFromId(GameObjectTypes::PlayerController);
    clientToPlayerController[clientId] = static_cast<ServerPlayerController*>(pair.first);

    return pair.first->GetNetworkId();
}

ReplicatedObjectPair ServerReplicationManager::CreateReplicatedObject(GameObjectTypes type, uint32 networkId) const
{
    ReplicatedObjectPair result;

    switch (type)
    {
    case(GameObjectTypes::Entity):
    case(GameObjectTypes::EntityController):
        result = ReplicatedObjectPair(std::make_unique<EntityController>(), std::make_unique<Entity>());
        break;

    case(GameObjectTypes::PlayerController):
        result = ReplicatedObjectPair(std::make_unique<ServerPlayerController>(), std::make_unique<ServerTank>());
        break;

    case(GameObjectTypes::TankController):
        result = ReplicatedObjectPair(std::make_unique<TankController>(), std::make_unique<Tank>());
        break;

    case(GameObjectTypes::Bullet):
    case(GameObjectTypes::BulletController):
        result = ReplicatedObjectPair(std::make_unique<BulletController>(), std::make_unique<ServerBullet>());
        break;

    default:
        assertNoEntry(); // The given type is not supported by the factory or not replicated.
        break;
    }

    if (result.first)
    {
        result.first->SetNetworkId(networkId == NETWORK_ID_NONE ? GenerateNetworkId() : networkId);
    }

    // We have received a object with a networkId higher than we expected. Try to resolve the issue by matching the id.
    if (nextNetworkid < networkId)
    {
        nextNetworkid = networkId;
    }

    return result;
}

void ServerReplicationManager::RemoveMatchingCreateMessage(DestroyObjectMessage* destroyMessage)
{
    assert(destroyMessage);

    for (size_t i = 0; i < criticalMessages.size(); ++i)
    {
        if ((ReliableMessageType)criticalMessages.at(i)->type == ReliableMessageType::CreateObject)
        {
            const auto& it = criticalMessages.at(i);
            CreateObjectMessage* createMessage = static_cast<CreateObjectMessage*>(it.get());

            if (destroyMessage->networkId == createMessage->networkId)
            {
                criticalMessages.erase(criticalMessages.begin() + i);
                return;
            }
        }
    }
}

void ServerReplicationManager::ReceiveInputMessage(const meteor::payload_packet& payload, byte_stream_reader& reader, uint32 clientId)
{
    input_message message;
    if (!message.read(reader))
    {
        debug::warn("Couldn't read input message from client!");
        return;
    }

    if (clientToPlayerController.find(clientId) == clientToPlayerController.end())
    {
        debug::warn("Input received for non-existing client & PlayerController. Dropping input.");
        return;
    }

    ServerPlayerController* playerController = clientToPlayerController[clientId];

    if (!playerController)
    {
        assertNoEntry(); // Clients PlayerController is null? Was it removed / destroyed by gameplay code?
        return;
    }

    playerController->ReceiveClientInput(message);
}

void ServerReplicationManager::ReceiveReliableMessages(const meteor::payload_packet& payload, byte_stream_reader& reader, uint32 clientId)
{
    // Start of messages found. This is always put at the end of the stream, so we can stop reading after this.
    if (clientToMessagingSystem.find(clientId) == clientToMessagingSystem.end())
    {
        debug::warn("Message received for non-existing client. Dropping messages.");
        return;
    }

    clientToMessagingSystem[clientId].ReceiveMessageStream(reader, payload);
}

