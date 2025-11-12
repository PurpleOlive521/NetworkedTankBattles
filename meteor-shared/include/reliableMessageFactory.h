#pragma once

#include "reliableMessagingSystem.h"
#include "eventSystemTypes.h"

class ReplicationManager;

class ReliableMessageFactory
{
public:
	ReliableMessageFactory() = delete;

	static ReliableMessagePtr CreateMessage(ReliableMessageType type);

	// Converts between a ReliableMessage and the appropriate Event. Returns nullptr if no conversion is possible.
	static ReliableMessagePtr CreateMessageFromEvent(EventBase* inEvent, ReplicationManager* replicationManager);
};