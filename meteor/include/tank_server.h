#pragma once

#include "tank.h"

class ServerTank final : public Tank
{
public:

	ServerTank() = default;

	bool TryShoot() override;
};