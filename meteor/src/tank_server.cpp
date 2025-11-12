#include "tank_server.h"
#include "game.h"
#include "eventFactory.h"

bool ServerTank::TryShoot()
{
	if (!Tank::TryShoot())
	{
		return false;
	}

	EventBasePtr event = EventFactory::CreateEvent(EventType::TankFired);
	TankFiredEvent* tankFiredEvent = static_cast<TankFiredEvent*>(event.get());
	tankFiredEvent->direction = turretRotation;
	const Vector2 outPosition = GetPosition();
	tankFiredEvent->position = outPosition;
	tankFiredEvent->shooter = this;

	owningGame->GetEventSystem()->QueueEvent(std::move(event));

	return true;
}
