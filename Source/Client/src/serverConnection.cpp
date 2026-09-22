#include "serverConnection.h"
#include "raylib.h"
#include "constants_shared.h"
#include "messages.h"
#include "application_client.h"
#include <format>
#include "playerController_client.h"

namespace meteor
{
	ServerConnection::ServerConnection(ip_endpoint inEndpoint, network_layer* inNetworkLayer, Client* inOwningClient) : Connection(inEndpoint, inNetworkLayer)
	{
		assert(inOwningClient);
		owningClient = inOwningClient;
		reconnectTimer.duration = RECONNECT_PACKET_INTERVAL;
	}

	void ServerConnection::ReceiveConnectPacket(connect_packet& packet, ip_endpoint inEndpoint)
	{
		// If we haven't initiated the connection process drop the packet.
		if (status != Status::CONNECTING)
		{
			return;
		}

		if (inEndpoint == owningClient->clientEndpoint)
		{
			return;
		}

		endpoint = inEndpoint;

		connectTimestamp = packet.m_send_time - GetTime();

		status = Status::CONNECTED;

		ResetTimeoutTimer();
	}

	void ServerConnection::ReceivePayloadPacket(const payload_packet& packet, byte_stream_reader& reader)
	{
		if (status != Status::CONNECTED)
		{
			return;
		}

		// Out of order packet, drop it
		if(packet.m_sequence <= acknowledge)
		{
			return;
		}

		acknowledge = packet.m_sequence;
		receivedAcknowledge = packet.m_acknowledge;
		ResetTimeoutTimer();

		// --- All checks passed and state is updated, hand over replication and gameplay events to any listener

		if (owningClient->GetListener())
		{
			owningClient->GetListener()->OnReceiveServerMessages(packet, reader);
		}
	}

	void ServerConnection::ReceiveDisconnectPacket(disconnect_packet& packet)
	{
		if(status == Status::DISCONNECTED)
		{
			return;
		}

		if (status == Status::DISCONNECTING)
		{
			status = Status::DISCONNECTED;
			ResetTimeoutTimer();
			reconnectTimer.Restart();
			return;
		}
		
		// If we receive a disconnect packet while being CONNECTED, we assume the disconnect was not initiated by us and should therefore immediately go to DISCONNECTED
		status = Status::DISCONNECTED;

		// Send the packet back

		switch ((disconnect_reason_type)packet.m_reason) {
		case disconnect_reason_type::WRONG_VERSION: 
		{
			disconnectReason = "Version Mismatch";
			break;
		}
		case disconnect_reason_type::TIMEOUT: 
		{
			disconnectReason = "Connection Timed Out";
			break;
		}
		case disconnect_reason_type::SERVER_CLOSE:
		{
			disconnectReason = "Server Closed";
			break;
		}
		case disconnect_reason_type::CUSTOM_REASON: 
		{
			disconnectReason = packet.m_message; 
		}
		}

		SendDisconnectPacket((disconnect_reason_type)packet.m_reason, sharedConstants::CRITICAL_MESSAGE_SEND_COUNT);

		reconnectTimer.Restart();
		ResetTimeoutTimer();
	}

	void ServerConnection::ReceivePingPacket(ping_packet& packet)
	{
		if (status != Status::CONNECTED)
		{
			return;
		}

		rtt = (GetTime() - packet.m_send_time) * 1000;
	}


	bool ServerConnection::SendPayloadPacket(byte_stream byteStream)
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
		lastSendTime = GetTime();

		return true;
	}

	bool ServerConnection::SendDisconnectPacket(disconnect_reason_type reason, int sendCount)
	{
		if (status == Status::DISCONNECTED || status == Status::CONNECTING)
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
		lastSendTime = GetTime();

		return bSuccess;
	}

	bool ServerConnection::SendPingPacket(ping_packet& pingPacket)
	{
		if (status != Status::CONNECTED)
		{
			return false; 
		}

		outStream.reset();
		byte_stream_writer writer(outStream);

		pingPacket.m_send_time = GetTime();
		pingPacket.write(writer);

		if (!SendPacketTo(endpoint, outStream))
		{
			debug::warn("Ping packet couldn't be sent!");
			return false;
		}

		return true;
	}

	bool ServerConnection::SendServerQueryPacket(server_query_packet& serverQueryPacket, int sendCount)
	{
		ip_address broadcastAddress = owningClient->clientEndpoint.address();
		//broadcastAddress.c(255); // Broadcast level for eduroam
		broadcastAddress.d(255); // Broadcast level

		ip_endpoint broadcastEndpoint = ip_endpoint(broadcastAddress, sharedConstants::DEFAULT_PORT);

		outStream.reset();
		byte_stream_writer writer(outStream);

		serverQueryPacket.write(writer);

		bool bSuccess = true;
		for (int i = 0; i < sendCount; i++)
		{
			bSuccess &= SendPacketTo(broadcastEndpoint, outStream);
		}

		return bSuccess;
	}

	void ServerConnection::Disconnect(disconnect_reason_type reason)
	{
		if (status == Status::DISCONNECTED)
		{
			return;
		}

		SendDisconnectPacket(reason, sharedConstants::CRITICAL_MESSAGE_SEND_COUNT);
		
		status = Status::DISCONNECTING;
		ResetTimeoutTimer();
	}

	void ServerConnection::Timeout()
	{
		SendDisconnectPacket(disconnect_reason_type::TIMEOUT, sharedConstants::CRITICAL_MESSAGE_SEND_COUNT);

		status = Status::DISCONNECTED;

		debug::info("Client disconnected due to timeout.");
	}

	std::string ServerConnection::GetDebugString() const
	{
		std::string parentString = Connection::GetDebugString();

		const std::string endLine = { ENDLINE };
		const std::string spacer = { "   " };

		parentString += spacer + std::string(" Rtt: ") + std::format("{:.2f}", rtt) + std::string("ms") + endLine;

		return parentString;
	}

	byte_stream ServerConnection::GetPayloadStream(payload_packet& outHeader)
	{
		outStream.reset();
		byte_stream_writer writer(outStream);

		payload_packet payloadPacket;
		payloadPacket.m_sequence = sequence++;
		payloadPacket.m_acknowledge = acknowledge;
		payloadPacket.write(writer);

		outHeader = payloadPacket;

		return outStream;
	}

	bool ServerConnection::ConnectToServer()
	{
		ResetTimeoutTimer();
		sendTimer.Start(constants::SEND_INTERVAL);
		connectTimer.Start(CONNECT_PACKET_INTERVAL);

		server_query_packet serverQueryPacket;
		if (!SendServerQueryPacket(serverQueryPacket, 10))
		{
			connectTimer.Stop();
			debug::warn("Server connect burst not sent successfully. Retrying next update.");
			return false;
		}

		status = Status::CONNECTING;
		return true;
	}

	void ServerConnection::Update(float deltaTime)
	{
		if (!reconnectTimer.IsFinished())
		{
			return;
		}

		const bool bShouldConnect = status == Status::DISCONNECTED || status == Status::CONNECTING;
		if (reconnectTimer.IsFinished() && bShouldConnect)
		{
			if (connectTimer.IsFinished()) 
			{
				sendTimer.Restart();
				connectTimer.Restart();

				if (!ConnectToServer()) 
				{
					debug::warn("Couldn't send connect packet!");
				}
			}
		}

		if (sendTimer.IsFinished() && status == Status::CONNECTED)
		{
			sendTimer.Restart();
			
			ping_packet pingPacket;
			SendPingPacket(pingPacket);

			// Write input state
			payload_packet header;
			byte_stream stream = GetPayloadStream(header);
			byte_stream_writer writer(stream);

			ClientReplicationManager* replicationManager = owningClient->owningApplication->GetClientReplicationManager();

			replicationManager->GetInputWithMessages(header, writer);

			if (!SendPayloadPacket(stream))
			{
				return;
			}
			
		}

	}

}
