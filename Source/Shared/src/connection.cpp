#include "connection.h"
#include "raylib.h"
#include "timer.h"
#include "constants_shared.h"

namespace meteor
{
	uint32 Connection::ID_COUNTER = 1;

	uint32 Connection::GenerateId()
	{
		uint32 new_id = ID_COUNTER;
		
		if (ID_COUNTER >= UINT32_MAX)
		{
			ID_COUNTER = 0;
		}
		else
		{
			ID_COUNTER++;
		}

		return new_id;
	}

	Connection::Connection(ip_endpoint inEndpoint, network_layer* inNetworkLayer)
	{
		id = GenerateId();
		endpoint = inEndpoint;
		status = Status::DISCONNECTED;
		lastReceivedTime = GetTime();

		assert(inNetworkLayer);
		networkLayer = inNetworkLayer;
	}

	void Connection::ReceiveConnectPacket(connect_packet& packet, ip_endpoint inEndpoint)
	{

	}

	void Connection::ReceivePayloadPacket(const payload_packet& packet, byte_stream_reader& reader)
	{

	}

	void Connection::ReceiveDisconnectPacket(disconnect_packet& packet)
	{

	}

	void Connection::Update(float deltaTime)
	{

	}

	bool Connection::SendConnectPacket(connect_packet& connectPacket, int sendCount)
	{
		return true;
	}

	bool Connection::SendPayloadPacket(byte_stream byteStream)
	{
		return true;
	}

	bool Connection::SendDisconnectPacket(disconnect_reason_type reason, int sendCount)
	{
		return true;
	}

	void Connection::Disconnect(disconnect_reason_type reason)
	{

	}

	void Connection::Timeout()
	{

	}

	bool Connection::SendPacketTo(const ip_endpoint& inEndpoint, const byte_stream& stream)
	{
		const float percentage = GetRandomValue(1, 100) / 100.0f;

		if (percentage <= sharedConstants::SIMULATED_PACKET_LOSS)
		{
			return true;
		}

		if (!networkLayer)
		{
			debug::error("Trying to send packet with invalid networkLayer!");
			return false;
		}

		return networkLayer->send_to(inEndpoint, stream);
	}

	byte_stream Connection::GetPayloadStream(payload_packet& outHeader)
	{
		outStream.reset();
		byte_stream_writer writer(outStream);

		payload_packet payloadPacket;
		payloadPacket.m_sequence = sequence++;
		payloadPacket.m_acknowledge = acknowledge;
		payloadPacket.m_send_time = GetTime();
		payloadPacket.write(writer);

		outHeader = payloadPacket;

		return outStream;
	}

	bool Connection::IsTimedOut() const
	{
		if (status == Status::DISCONNECTED) 
		{
			return false;
		}

		return GetTime() - lastReceivedTime >= CONNECTION_TIMEOUT;
	}

	bool Connection::IsActive() const
	{
		return status != Status::DISCONNECTED;
	}

	std::string Connection::GetStatusAsString() const
	{
		switch (status)
		{
			case(Status::DISCONNECTED):
				return "DISCONNECTED";
				break;
			case(Status::CONNECTING):
				return "CONNECTING";
				break;
			case(Status::CONNECTED):
				return "CONNECTED";
				break;
			case(Status::DISCONNECTING):
				return "DISCONNECTING";
				break;
		}

		return std::string("Invalid status!");
	}

	std::string Connection::GetDebugString() const
	{
		const std::string endLine = { ENDLINE };
		const std::string spacer = { "   " };

		std::string debugString = {};
		debugString += std::string("Id: ") + std::to_string(id) + endLine;
		debugString += spacer + GetStatusAsString() + endLine;
		debugString += spacer + std::string(" Seq: ") + std::to_string(sequence) + endLine;
		debugString += spacer + std::string(" Ack: ") + std::to_string(acknowledge) + endLine;

		return debugString;
	}

	void Connection::ResetTimeoutTimer()
	{
		lastReceivedTime = GetTime();
	}

}
