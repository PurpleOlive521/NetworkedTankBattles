#include "reliableMessageFactoryTypes.h"

template <typename T>
bool Serialize(ReliableMessage& message, T& stream)
{
    bool success = true;

	success &= stream.serialize(message.type);
	success &= stream.serialize(message.id);

    return success;
}

ReliableMessage::~ReliableMessage()
{
}

bool ReliableMessage::Write(meteor::byte_stream_writer& writer)
{
	return Serialize(*this, writer);
}

bool ReliableMessage::Read(meteor::byte_stream_reader& reader)
{
	return Serialize(*this, reader);
}

uint32 ReliableMessage::Measure(meteor::byte_stream_measurer& measurer)
{
	return Serialize(*this, measurer);
}

void ReliableMessage::AssignId(uint32 inId)
{
	id = inId;
}




CreateObjectMessage::CreateObjectMessage()
{
	type = (uint8)ReliableMessageType::CreateObject;
	bIsCritical = true;
}

template <typename T>
bool Serialize(CreateObjectMessage& message, T& stream)
{
	bool success = true;

	success &= stream.serialize(message.networkId);
	success &= stream.serialize(message.classId);

	return success;
}

bool CreateObjectMessage::Write(meteor::byte_stream_writer& writer)
{
	if (!ReliableMessage::Write(writer))
	{
		return false;
	}

	return Serialize(*this, writer);
}

bool CreateObjectMessage::Read(meteor::byte_stream_reader& reader)
{
	if (!ReliableMessage::Read(reader))
	{
		return false;
	}

	return Serialize(*this, reader);
}

uint32 CreateObjectMessage::Measure(meteor::byte_stream_measurer& measurer)
{
	if (!ReliableMessage::Measure(measurer))
	{
		return 0U;
	}

	return Serialize(*this, measurer);
}




DestroyObjectMessage::DestroyObjectMessage()
{
	type = (uint8)ReliableMessageType::DestroyObject;
}

template <typename T>
bool Serialize(DestroyObjectMessage& message, T& stream)
{
	bool success = true;

	success &= stream.serialize(message.networkId);

	return success;
}

bool DestroyObjectMessage::Write(meteor::byte_stream_writer& writer)
{
	if (!ReliableMessage::Write(writer))
	{
		return false;
	}

	return Serialize(*this, writer);
}

bool DestroyObjectMessage::Read(meteor::byte_stream_reader& reader)
{
	if (!ReliableMessage::Read(reader))
	{
		return false;
	}

	return Serialize(*this, reader);
}

uint32 DestroyObjectMessage::Measure(meteor::byte_stream_measurer& measurer)
{
	if (!ReliableMessage::Measure(measurer))
	{
		return 0U;
	}

	return Serialize(*this, measurer);
}



TankFiredMessage::TankFiredMessage()
{
}

template <typename T>
bool Serialize(TankFiredMessage& message, T& stream)
{
	bool success = true;

	success &= stream.serialize(message.networkId);

	return success;
}

bool TankFiredMessage::Write(meteor::byte_stream_writer& writer)
{
	if (!ReliableMessage::Write(writer))
	{
		return false;
	}

	return Serialize(*this, writer);
}

bool TankFiredMessage::Read(meteor::byte_stream_reader& reader)
{
	if (!ReliableMessage::Read(reader))
	{
		return false;
	}

	return Serialize(*this, reader);
}

uint32 TankFiredMessage::Measure(meteor::byte_stream_measurer& measurer)
{
	if (!ReliableMessage::Measure(measurer))
	{
		return 0U;
	}

	return Serialize(*this, measurer);
}
