#include "eventSystem.h"

void EventSystem::ProcessEventQueue()
{
	while (!eventQueue.empty())
	{
		EventBasePtr& front = eventQueue.front();
		EventBase* eventToProcess = front.get();

		for (auto [it, itEnd] = eventReplies.equal_range(eventToProcess->type); it != itEnd; ++it)
		{
			it->second(eventToProcess);
		}

		eventQueue.pop();
	}
}

void EventSystem::ClearEventQueue()
{
	eventReplies.clear();
}

void EventSystem::AddEventReply(EventType type, EventDelegate eventReply)
{
	assert(eventReply);

	if (type >= EventType::ENUM_MAX)
	{
		debug::warn("Invalid event type added as reply! Possible malformed data.");
		return;
	}

	eventReplies.insert({ type, eventReply });
}

void EventSystem::QueueEvent(EventBasePtr event)
{
	assert(event);

	EventBase* out = event.get();
	eventQueue.emplace(std::move(event));

	if (listener)
	{
		listener->OnEventQueued(out);
	}
}

void EventSystem::AddListener(EventListener* inListener)
{
	listener = inListener;
}
