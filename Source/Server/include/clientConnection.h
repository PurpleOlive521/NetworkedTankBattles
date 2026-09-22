#pragma once

#include "network_layer.h"
#include "protocol.h"
#include "connection.h"


namespace meteor
{
	class Server;

	// Represents a connection from the server to a client.
	class ClientConnection final: public Connection
	{
	public:
		ClientConnection() = default;
		ClientConnection(ip_endpoint endpoint, network_layer* networkLayer, Server* inOwningServer); // Set up as status::CONNECTING

		// --- Receive packets

		void ReceiveConnectPacket(connect_packet& packet, ip_endpoint inEndpoint) override;
		
		void ReceivePayloadPacket(const payload_packet& packet, byte_stream_reader& reader) override;

		void ReceiveDisconnectPacket(disconnect_packet& packet) override;

		void ReceivePingPacket(ping_packet& packet);

		void ReceiveServerQueryPacket();

		// --- Send packets

		bool SendConnectPacket(connect_packet& connectPacket, int sendCount = 1) override;

		bool SendPayloadPacket(byte_stream byteStream) override;

		bool SendDisconnectPacket(disconnect_reason_type reason, int sendCount = 1) override;

		void SendPingPacket(ping_packet& packet);

		void Disconnect(disconnect_reason_type reason) override;

		void Timeout() override;

		// NetworkId of the player controller associated with this client.
		uint32 playerControllerId = 0;

		// The server that owns this connection. Can be nullptr.
		Server* owningServer = nullptr;

	};
}