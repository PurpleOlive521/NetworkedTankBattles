#pragma once

#include "entityController.h"
#include "tank.h"

struct TankState
{
	TankState() = default;
	TankState(Tank* inTank);

	bool Write(meteor::byte_stream_writer& writer);
	bool Read(meteor::byte_stream_reader& reader);

	// Applies the stored state on the Tank. Position and rotation is expected to be interpolated and is not applied.
	void ApplyOnInterpolatedTank(Tank* inTank) const;

	void ApplyToTank(Tank* inTank) const;

	Vector2 position = {};
	Vector2 aimTarget = {};
	float rotation = 0.0f;
	float turretRotation = 0.0f;
	bool bIsDead = false;
};

class TankController : public EntityController
{
public:
	TankController() = default;

	virtual GameObjectTypes GetType() const override;

	virtual void OnControlledEntityChanged() override;

	virtual bool Write(meteor::byte_stream_writer& writer) override;

	virtual bool Read(meteor::byte_stream_reader& reader) override;

protected:
	Tank* controlledTank = nullptr;
};