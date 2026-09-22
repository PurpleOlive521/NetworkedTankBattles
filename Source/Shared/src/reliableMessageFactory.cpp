#include "reliableMessageFactory.h"
#include "replicationManager.h"

ReliableMessagePtr ReliableMessageFactory::CreateMessage(ReliableMessageType type)
{
	ReliableMessagePtr outMessage = nullptr;

	switch (type)
	{
	case ReliableMessageType::CreateObject:
		outMessage = std::make_shared<CreateObjectMessage>();
		break;

	case ReliableMessageType::DestroyObject:
		outMessage = std::make_shared<DestroyObjectMessage>();
		break;

	case ReliableMessageType::TankFired:
		outMessage = std::make_shared<TankFiredMessage>();
		break;

	default:
		assertNoEntry(); // ReliableMessageType not supported yet.
	}

	return outMessage;
}

ReliableMessagePtr ReliableMessageFactory::CreateMessageFromEvent(EventBase* inEvent, ReplicationManager* replicationManager)
{
	assert(inEvent);

	switch (inEvent->type)
	{
	case EventType::TankFired:
		ReliableMessagePtr message = CreateMessage(ReliableMessageType::TankFired);
		TankFiredMessage* tankFiredMessage = static_cast<TankFiredMessage*>(message.get());

		TankFiredEvent* tankFiredEvent = static_cast<TankFiredEvent*>(inEvent);

		tankFiredMessage->direction = tankFiredEvent->direction;
		tankFiredMessage->position = tankFiredEvent->position;
		tankFiredMessage->networkId = replicationManager->GetNetworkIdOfObject(tankFiredEvent->shooter);
		return message;
	}		

	return nullptr;
}
