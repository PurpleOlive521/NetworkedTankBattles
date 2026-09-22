#pragma once

#include "core.h"
#include "network.h"
#include "raylib.h"

enum class ReliableMessageType : uint8
{
	CreateObject,
	DestroyObject,
	TankFired,

	ENUM_MAX
};

struct ReliableMessage
{
	ReliableMessage() = default;
	virtual ~ReliableMessage();

	virtual bool Write(meteor::byte_stream_writer& writer);

	virtual bool Read(meteor::byte_stream_reader& reader);

	virtual uint32 Measure(meteor::byte_stream_measurer& measurer);

	// Assigns the message id. Not to be confused with networkId!
	void AssignId(uint32 inId);

	uint8 type = (uint8)ReliableMessageType::ENUM_MAX;

	uint32 id = 0U;

	// Static per message, not replicated.
	bool bIsCritical = false;
};

struct CreateObjectMessage final: public ReliableMessage
{
	CreateObjectMessage();

	bool Write(meteor::byte_stream_writer& writer) override;

	bool Read(meteor::byte_stream_reader& reader) override;

	uint32 Measure(meteor::byte_stream_measurer& measurer) override;

	uint32 networkId = 0;

	uint8 classId = 0U;
};

struct DestroyObjectMessage final : public ReliableMessage
{
	DestroyObjectMessage();

	bool Write(meteor::byte_stream_writer& writer) override;

	bool Read(meteor::byte_stream_reader& reader) override;

	uint32 Measure(meteor::byte_stream_measurer& measurer) override;

	uint32 networkId = 0;
};

struct TankFiredMessage final : public ReliableMessage
{
	TankFiredMessage();

	bool Write(meteor::byte_stream_writer& writer) override;

	bool Read(meteor::byte_stream_reader& reader) override;

	uint32 Measure(meteor::byte_stream_measurer& measurer) override;

	uint32 networkId = 0; // The controller of the object that fired

	Vector2 position = {};

	float direction = 0.0f;
};

