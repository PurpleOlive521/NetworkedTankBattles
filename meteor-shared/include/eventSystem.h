#pragma once

#include <queue>
#include <map>
#include <functional>
#include "eventSystemTypes.h"
#include <memory>

typedef std::function<void(EventBase*)> EventDelegate;

typedef std::unique_ptr<EventBase> EventBasePtr;

class EventSystem
{
public:
	class EventListener
	{
	public:

		virtual void OnEventQueued(EventBase* event) = 0;
	};

	EventSystem() = default;

	void ProcessEventQueue();

	void ClearEventQueue();

	void AddEventReply(EventType type, EventDelegate eventReply);

	void QueueEvent(EventBasePtr event);

	void AddListener(EventListener* inListener);

private:

	std::queue<EventBasePtr> eventQueue;

	std::multimap<EventType, EventDelegate> eventReplies;

	EventListener* listener = nullptr;
};