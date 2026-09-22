#pragma once

#include "network_layer.h"
#include "protocol.h"
#include "connection.h"
#include "timer.h"


namespace meteor
{
	class Client; 

	constexpr float RECONNECT_PACKET_INTERVAL = 5.f;
	constexpr float CONNECT_PACKET_INTERVAL = 1.f;

	class ServerConnection final : public Connection
	{
	public:
		ServerConnection() = default;
		ServerConnection(ip_endpoint inEndpoint, network_layer* inNetworkLayer, Client* inOwningClient); // Set up as status::DISCONNECTED

		// --- Receive packets

		void ReceiveConnectPacket(connect_packet& packet, ip_endpoint inEndpoint) override;
		void ReceivePayloadPacket(const payload_packet& packet, byte_stream_reader& reader) override;
		void ReceiveDisconnectPacket(disconnect_packet& packet) override;
		void ReceivePingPacket(ping_packet& packet);
		
		// --- Send packets

		void Update(float deltaTime);

		bool SendPayloadPacket(byte_stream byteStream) override;
		bool SendDisconnectPacket(disconnect_reason_type reason, int sendCount = 1) override;
		bool SendPingPacket(ping_packet& pingPacket);
		bool SendServerQueryPacket(server_query_packet& serverQueryPacket, int sendCount = 1);

		void Disconnect(disconnect_reason_type reason) override;

		void Timeout() override;

		std::string GetDebugString() const override;

		byte_stream GetPayloadStream(payload_packet& outHeader) override;

		// Returns true if the client successfully sends a connection packet
		bool ConnectToServer();

		double connectTimestamp = 0; // Servers time at the point of connecting.
		double lastSendTime = 0; //Last time when a packet was send - used for calculating RTT
		double rtt = 0; // Round-trip time in seconds.


		Timer sendTimer;
		Timer reconnectTimer;
		Timer connectTimer;

		Client* owningClient = nullptr;

		const char* disconnectReason; 
	};
}