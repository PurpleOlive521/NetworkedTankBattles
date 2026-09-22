#include "clientConnection.h"
#include "raylib.h"
#include "constants_shared.h"
#include "server.h"

namespace meteor
{
	ClientConnection::ClientConnection(ip_endpoint inEndpoint, network_layer* inNetworkLayer, Server* inOwningServer) : Connection(inEndpoint, inNetworkLayer)
	{
		assert(inOwningServer);

		owningServer = inOwningServer;
	}

	void ClientConnection::ReceiveConnectPacket(connect_packet& packet, ip_endpoint inEndpoint)
	{
		// We are not disconnected, drop the packet
		if (status != Status::DISCONNECTED)
		{
			return;
		}

		status = Status::CONNECTING;
		ResetTimeoutTimer();

		packet.m_send_time = GetTime();
		SendConnectPacket(packet);
	}

	void ClientConnection::ReceivePayloadPacket(const payload_packet& packet, byte_stream_reader& reader)
	{
		if (status == Status::DISCONNECTED || status == Status::DISCONNECTING)
		{
			debug::info("Payload packet received from already disconnected client.");
			return;
		}

		if (status == Status::CONNECTING)
		{
			debug::info("New client now connected!");

			// The server should now create a PlayerController for us and return it's networkId.
			if (owningServer->GetListener())
			{
				playerControllerId = owningServer->GetListener()->OnClientConnected(id);
			}

			status = Status::CONNECTED;
		}

		// Out of order packet, drop it
		if(packet.m_sequence <= acknowledge)
		{
			debug::info("Out of order payload received. Dropping.");
			return;
		}

		acknowledge = packet.m_sequence;
		receivedAcknowledge = packet.m_acknowledge;

		ResetTimeoutTimer();

		// --- All checks passed and state is updated, hand over the payload contents to any listener

		if (owningServer->GetListener())
		{
			owningServer->GetListener()->OnReceiveClientMessages(packet, reader, id);
		}
	}

	void ClientConnection::ReceiveDisconnectPacket(disconnect_packet& packet)
	{
		if(status == Status::DISCONNECTED)
		{
			return;
		}

		if (status == Status::DISCONNECTING)
		{
			debug::info("Client disconnected per their request.");
			status = Status::DISCONNECTED;
			ResetTimeoutTimer();

			if (owningServer->GetListener())
			{
				owningServer->GetListener()->OnClientDisconnected(id, false);
			}

			return;
		}
		
		// Send the packet back
		for (int i = 0; i < 10; i++)
		{
			SendDisconnectPacket((disconnect_reason_type)packet.m_reason);
		}

		status = Status::DISCONNECTING;
		ResetTimeoutTimer();
	}

	void ClientConnection::ReceivePingPacket(ping_packet& packet)
	{
		if (status != Status::CONNECTED)
		{
			return;
		}

		SendPingPacket(packet);
	}

	void ClientConnection::ReceiveServerQueryPacket()
	{
		// We are not disconnected, drop the packet
		if (status != Status::DISCONNECTED)
		{
			return;
		}

		status = Status::CONNECTING;
		ResetTimeoutTimer();
		
		connect_packet packet;
		SendConnectPacket(packet, 10);
	}

	bool ClientConnection::SendConnectPacket(connect_packet& connectPacket, int sendCount)
	{
		outStream.reset();
		byte_stream_writer writer(outStream);

		connectPacket.m_send_time = GetTime(); 
		connectPacket.write(writer);

		bool bSuccess = true;
		for(int i = 0; i < sendCount; i++)
		{
			bSuccess &= SendPacketTo(endpoint, outStream);
		}

		return bSuccess;
	}

	bool ClientConnection::SendPayloadPacket(byte_stream byteStream)
	{
		if (status == Status::DISCONNECTED || status == Status::DISCONNECTING)
		{
			return false;
		}

		if (!SendPacketTo(endpoint, byteStream))
		{
			debug::warn("Payload couldn't be sent!");
			return false;
		}

		return true;
	}

	bool ClientConnection::SendDisconnectPacket(disconnect_reason_type reason, int sendCount)
	{
		if (status == Status::DISCONNECTED)
		{
			return false;
		}

		outStream.reset();
		byte_stream_writer writer(outStream);

		disconnect_packet packet(reason);

		packet.write(writer);

		bool bSuccess = true;
		for (int i = 0; i < sendCount; i++)
		{
			bSuccess &= SendPacketTo(endpoint, outStream);
		}

		return bSuccess;
	}

	void ClientConnection::SendPingPacket(ping_packet& packet)
	{
		if (status != Status::CONNECTED)
		{
			return;
		}

		outStream.reset();
		byte_stream_writer writer(outStream);
		packet.write(writer);
		
		if (!SendPacketTo(endpoint, outStream))
		{
			debug::warn("Couldn't send ping packet to Client");
			return;
		}
	
	}

	void ClientConnection::Disconnect(disconnect_reason_type reason)
	{
		debug::info("Client disconnect requested.");

		SendDisconnectPacket(reason, sharedConstants::CRITICAL_MESSAGE_SEND_COUNT);

		status = Status::DISCONNECTING;
		ResetTimeoutTimer();

		if (owningServer->GetListener())
		{
			owningServer->GetListener()->OnClientDisconnected(id, false);
		}
	}

	void ClientConnection::Timeout()
	{
		SendDisconnectPacket(disconnect_reason_type::TIMEOUT, sharedConstants::CRITICAL_MESSAGE_SEND_COUNT);

		status = Status::DISCONNECTED;

		debug::info("Client disconnected due to timeout.");

		if (owningServer->GetListener())
		{
			owningServer->GetListener()->OnClientDisconnected(id, true);
		}
	}
}
