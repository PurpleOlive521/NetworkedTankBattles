#pragma once

#include "network_layer.h"
#include "protocol.h"
#include <string>

namespace meteor
{
	constexpr double CONNECTION_TIMEOUT = 5.0; // Seconds

	// Derive and build the server/client specific functionality ontop of this.
	class Connection
	{
	public:
		enum class Status
		{
			DISCONNECTED,
			CONNECTING,
			CONNECTED,
			DISCONNECTING,
		};

		Connection() = default;
		Connection(ip_endpoint inEndpoint, network_layer* inNetworkLayer); // Set up as status::CONNECTING

		// --- Receive packets

		virtual void ReceiveConnectPacket(connect_packet& packet, ip_endpoint inEndpoint);
		
		virtual void ReceivePayloadPacket(const payload_packet& packet, byte_stream_reader& reader);

		virtual void ReceiveDisconnectPacket(disconnect_packet& packet);

		// --- Send packets

		virtual void Update(float deltaTime);

		// Returns true if sent successfully. Sends the packet sendCount amount of times.
		virtual bool SendConnectPacket(connect_packet& connectPacket, int sendCount = 1);

		// Returns true if sent successfully.
		// Remember to insert a header in the packet first!
		virtual bool SendPayloadPacket(byte_stream byteStream);

		// Returns true if sent successfully. Sends the packet sendCount amount of times.
		virtual bool SendDisconnectPacket(disconnect_reason_type reason, int sendCount = 1);
		
		// Sets the status to DISCONNECTING and sends a disconnect packet with the given reason.
		virtual void Disconnect(disconnect_reason_type reason);
		
		// Sets the status as DISCONNECTED immediately and sends a TIMEOUT disconnect packet.
		virtual void Timeout();

		bool SendPacketTo(const ip_endpoint& inEndpoint, const byte_stream& stream);

		// --- Utility

		// Returns a byte_stream prepped with a payload header specific to this connection.
		virtual byte_stream GetPayloadStream(payload_packet& outHeader);

		bool IsTimedOut() const;

		bool IsActive() const;

		std::string GetStatusAsString() const;

		// Returns a string with debug information about the connection.
		virtual std::string GetDebugString() const;

		// Reusable stream for sending data. Remember to call Reset() before populating.
		byte_stream outStream;

		uint32 id = 0;
		Status status = Status::DISCONNECTED;
		ip_endpoint endpoint;
		double lastReceivedTime = 0;

		uint32 sequence = 1;				// The connections sequence.
		uint32 acknowledge = 0;				// Latest sequence received from other end of the connection. Not our own!
		uint32 receivedAcknowledge = 0;		// The latest sequence that has been acknowledged by the other end.

		network_layer* networkLayer = nullptr;

	protected:
		static uint32 ID_COUNTER;
		static uint32 GenerateId();

		void ResetTimeoutTimer();
	};
}