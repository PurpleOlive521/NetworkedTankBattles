#pragma once

#include "core.h"
#include "raylib.h"

class GameObject;

enum class EventType : uint8
{
	TankFired,

	ENUM_MAX
};

struct EventBase
{
	EventBase() = default;
	virtual ~EventBase();

	EventType type = EventType::TankFired;
};

struct TankFiredEvent final : public EventBase
{
	TankFiredEvent();

	Vector2 position = {};

	float direction = 0.0f;

	GameObject* shooter = nullptr;
};