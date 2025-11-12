#include "eventFactory.h"
#include "replicationManager.h"

EventBasePtr EventFactory::CreateEvent(EventType type)
{
    EventBasePtr outEvent = nullptr;

    switch (type)
    {
    case EventType::TankFired:
        outEvent = std::make_unique<TankFiredEvent>();
        break;

    default:
        assertNoEntry(); // EventType not supported yet.
    }

    return outEvent;
}

EventBasePtr EventFactory::CreateEventFromMessage(ReliableMessage* inMessage, ReplicationManager* replicationManager)
{
    assert(inMessage);

    switch((ReliableMessageType)inMessage->type)
    {
    case ReliableMessageType::TankFired:
        EventBasePtr event = CreateEvent(EventType::TankFired);
        TankFiredEvent* tankFiredEvent = static_cast<TankFiredEvent*>(event.get());

        TankFiredMessage* tankFiredMessage = static_cast<TankFiredMessage*>(inMessage);

        tankFiredEvent->direction = tankFiredMessage->direction;
        tankFiredEvent->position = tankFiredMessage->position;
        Controller* controller = replicationManager->GetControllerByNetworkId(tankFiredMessage->networkId);

        if (controller)
        {
            tankFiredEvent->shooter = controller->GetControlledEntity();
        }

        return event;
    }

    return nullptr;
}
