#pragma once

#include "eventSystem.h"
#include "reliableMessageFactoryTypes.h"

class ReplicationManager;

class EventFactory
{
public:
	EventFactory() = delete;

	static EventBasePtr CreateEvent(EventType type);

	// Converts between a ReliableMessage and the appropriate Event. Returns nullptr if no conversion is possible.
	static EventBasePtr CreateEventFromMessage(ReliableMessage* inMessage, ReplicationManager* replicationManager);
};