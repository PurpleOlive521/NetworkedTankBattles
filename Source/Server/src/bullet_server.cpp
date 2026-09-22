#include "bullet_server.h"

#include "tank.h"

void ServerBullet::OnHitTank(Tank* other)
{
	assert(other);

	other->TriggerDeath();
	bPendingKill = true;
}

void ServerBullet::OnRicochet()
{
	ricochetsLeft--;
}
