#pragma once

#include "entityController.h"
#include "bullet.h"

class BulletController;

struct BulletState
{
	BulletState() = default;
	BulletState(Bullet* inBullet, BulletController* bulletController);

	bool Write(meteor::byte_stream_writer& writer);
	bool Read(meteor::byte_stream_reader& reader);

	void ApplyOnBullet(Bullet* inBullet, Game* owningGame);

	Vector2 position = {};
	float rotation = 0.0f;
	int remainingRicochets = 0;
	uint32 shooterId = 0U;
};

class BulletController : public EntityController
{
public:

	BulletController() = default;

	virtual GameObjectTypes GetType() const override;

	virtual void OnControlledEntityChanged() override;

	virtual bool Write(meteor::byte_stream_writer& writer) override;

	virtual bool Read(meteor::byte_stream_reader& reader) override;

protected:
	Bullet* controlledBullet = nullptr;
};