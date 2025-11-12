#include "eventSystemTypes.h"

TankFiredEvent::TankFiredEvent()
{
	type = EventType::TankFired;
}

EventBase::~EventBase()
{
}
