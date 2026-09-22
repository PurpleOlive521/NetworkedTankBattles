#include "controller.h"
#include "core.h"
#include "reliableMessageFactory.h"
#include "Entity.h"
#include "replicationManager.h"

void Controller::BeginPlay()
{
	GameObject::BeginPlay();
}

void Controller::Update(float DeltaTime)
{
	GameObject::Update(DeltaTime);
}

void Controller::OnTick()
{
}

void Controller::OnReplication(const meteor::payload_packet& payload)
{

}

bool Controller::Write(meteor::byte_stream_writer& writer)
{
	return false;
}

bool Controller::Read(meteor::byte_stream_reader& reader)
{
	return false;
}

GameObjectTypes Controller::GetType() const
{
	return GameObjectTypes::Controller;
}

void Controller::OnDestroy()
{
	// Make sure we don't leave a dangling object
	if (controlledEntity)
	{
		controlledEntity->SetIsPendingKill(true);
		controlledEntity = nullptr;
	}

	if (bGenerateDeathMessage)
	{
		// Send a message about this controller being destroyed
		ReliableMessagePtr message = ReliableMessageFactory::CreateMessage(ReliableMessageType::DestroyObject);
		DestroyObjectMessage* destroyedMessage = static_cast<DestroyObjectMessage*>(message.get());
		destroyedMessage->networkId = networkId;
		replicationManager->SendMessage(message);
	}

	// Finally, remove us from the ReplicationManager
	replicationManager->OnControllerDestroyed(GetNetworkId());
}

void Controller::OnControlledEntityChanged()
{
}

void Controller::SetControlledEntity(Entity* entity)
{
	if(controlledEntity == entity)
	{
		return;
	}

	controlledEntity = entity;
	controlledEntity->controller = this;
	OnControlledEntityChanged();
}

Entity* Controller::GetControlledEntity() const
{
	return controlledEntity;
}

void Controller::SetNetworkId(uint32 Id)
{
	networkId = Id;
}

uint32 Controller::GetNetworkId() const
{
	return networkId;
}

void Controller::SetReplicationManager(ReplicationManager* inManager)
{
	assert(inManager);
	replicationManager = inManager;
}
