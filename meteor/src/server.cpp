#include "server.h"
#include "constants_server.h"
#include "constants_shared.h"
#include "application_server.h"
#include "entity.h"

namespace meteor
{
    bool Server::Init(ServerApplication* inOwningApplication)
    {
        owningApplication = inOwningApplication;
        if (!owningApplication)
        {
            debug::error("Invalid owning application!");
            return false;
        }

        
        ip_endpoint serverEndpoint;
        if (network::query_local_addresses(addresses))
        {
            serverEndpoint = {addresses.front(), sharedConstants::DEFAULT_PORT};
        }

        if (!networkLayer.open(serverEndpoint))
        {
            debug::error("Could not open socket at server endpoint!");
            return false;
        }

        clients = ClientCollection(&networkLayer, this);

        debug::info("Server initialised successfully.");
        sendPacketTimer.Start(constants::SEND_INTERVAL);

        return true;
    }

    void Server::Update(float DeltaTime)
    {

        // Iterate through all incoming packets until the socket is empty
        std::function<void(ip_endpoint&, byte_stream&)> callback = std::bind(&Server::ProcessIncomingStream, this, std::placeholders::_1, std::placeholders::_2);
        while (networkLayer.receive(callback))
        {

        }

        // Send packet
        if (sendPacketTimer.IsFinished())
        {
            sendPacketTimer.Restart();

			// Send the game state to all connected clients
            for(ClientConnection& client : clients.connections)
            {
                if (client.IsActive())
                {
                    payload_packet header;
                    byte_stream outStream = client.GetPayloadStream(header);
                    byte_stream_writer writer(outStream);

                    const bool bSuccess = owningApplication->GetServerReplicationManager()->GetGameStateWithMessages(client.id, header, writer);

                    if(!bSuccess)
                    {
                        debug::warn("Failed to write game state packet!");
                        continue;
					}

                    if (!client.SendPayloadPacket(outStream))
                    {
                        debug::warn("Couldn't send to a client");
                        continue;
                    }
                }
			}
        }    

        // Tick timeout timers
        clients.UpdateCollection();
    }

    void Server::Render() const
    {
        std::string clientCountText = std::string("Server running with ") + std::to_string(clients.GetClientCount()) + std::string(" registered clients,");
        DrawText(clientCountText.c_str(), 10, 10, 24, RAYWHITE);

        std::string validClientCountText = std::string("of which ") + std::to_string(clients.GetClientCount()) + std::string(" are valid, active clients.");
        DrawText(validClientCountText.c_str(), 10, 40, 24, RAYWHITE);

		const int startY = 80;
		const int YOffset = 130;
        int count = 0;
        for (const ClientConnection& client : clients.connections)
        {
            DrawText(client.GetDebugString().c_str(), 10, startY + YOffset * count, 24, RAYWHITE);
            count++;
        }

        DrawFPS(GetScreenWidth() - 100, 0);
    }

    void Server::Exit()
    {
		// We send a disconnect packet to all clients before closing
		byte_stream outStream;
		byte_stream_writer writer(outStream);
		disconnect_packet packet(disconnect_reason_type::SERVER_CLOSE);
		packet.write(writer);

        // The call doesn't update the connections state, but it doesn't matter since we quit the application after this anyways.
		clients.SendToValidClients(outStream, sharedConstants::CRITICAL_MESSAGE_SEND_COUNT);

        networkLayer.close();
    }

    void Server::SetListener(Listener* inListener)
    {
		listener = inListener;
    }

    
    Server::Listener* Server::GetListener()
    {
        return listener;
    }

    void Server::ProcessIncomingStream(ip_endpoint& endpoint, byte_stream& stream)
    {
        // Empty packet, discard
        if (!stream.has_data())
        {
            debug::info("Empty packet received. Dropping packet.");
            return;
        }

        byte_stream_reader reader(stream);
        protocol_packet_type packetType = (protocol_packet_type)reader.peek();

        switch (packetType)
        {
        case protocol_packet_type::CONNECT:
            HandleConnect(endpoint, reader);
            return;

        case protocol_packet_type::PAYLOAD:
            HandlePayload(endpoint, reader);
            return;

        case protocol_packet_type::DISCONNECT:
            HandleDisconnect(endpoint, reader);
            return;

        case protocol_packet_type::PING:
            HandlePing(endpoint, reader);
            return;
        
        case protocol_packet_type::SERVER_QUERY:
            HandleServerQuery(endpoint, reader);
            return;
        }

    }

    void Server::HandleConnect(ip_endpoint& endpoint, byte_stream_reader& reader)
    {
        connect_packet packet;
        if (!packet.read(reader))
        {
            debug::warn("Failed to read connect packet!");
            return;
        }

        // Incorrect versioning, send back disconnect packet 
        if (packet.m_version != PROTOCOL_VERSION || packet.m_signature != PROTOCOL_SIGNATURE)
        {
            disconnect_packet response_packet(disconnect_reason_type::WRONG_VERSION);

            byte_stream out_stream;
            byte_stream_writer writer(out_stream);
            response_packet.write(writer);

            networkLayer.send_to(endpoint, out_stream);
            debug::warn("Incorrect packet version. Sending back disconnect packet.");
            return;
        }

        Connection clientConnection;

        const bool bNewClient = !clients.HasValidClient(endpoint);

        // Not a new client, drop the packet
        if (!bNewClient)
        {
            return;
        }

        debug::info("New client connecting.");

        Connection& client = clients.AddClient(endpoint);
        client.ReceiveConnectPacket(packet, endpoint);
    }

    void Server::HandlePayload(ip_endpoint& endpoint, byte_stream_reader& reader)
    {
        payload_packet packet;
        if (!packet.read(reader))
        {
            debug::warn("Failed to read payload packet!");
            return;
        }

        // Client not connected, drop packet
        if (!clients.HasValidClient(endpoint))
        {
            debug::info("Payload received from non-connected client.");
            return;
        }

        Connection& client = clients.GetValidClient(endpoint);
        client.ReceivePayloadPacket(packet, reader);
    }

    void Server::HandleDisconnect(ip_endpoint& endpoint, byte_stream_reader& reader)
    {
        disconnect_packet packet;
        if (!packet.read(reader))
        {
            debug::warn("Failed to read payload packet!");
            return;
        }

        // Client not connected, drop packet
        if (!clients.HasValidClient(endpoint))
        {
            return;
        }

        debug::info("Client disconnected.");

        Connection& client = clients.GetValidClient(endpoint);
        client.ReceiveDisconnectPacket(packet);
    }

    void Server::HandlePing(ip_endpoint& endpoint, byte_stream_reader& reader)
    {
        if (!clients.HasValidClient(endpoint))
        {
            return;
        }

        ping_packet packet; 
        if (!packet.read(reader))
        {
            debug::warn("Couldn't read ping message");
            return;
        }
        
        
        ClientConnection& client = clients.GetValidClient(endpoint);
        client.ReceivePingPacket(packet);
    }

    void Server::HandleServerQuery(ip_endpoint& endpoint, byte_stream_reader& reader)
    {
        server_query_packet packet;
        if (!packet.read(reader))
        {
            debug::warn("Failed to read server query packet!");
            return;
        }

        // Incorrect versioning, send back disconnect packet 
        if (packet.m_version != PROTOCOL_VERSION || packet.m_signature != PROTOCOL_SIGNATURE)
        {
            disconnect_packet response_packet(disconnect_reason_type::WRONG_VERSION);

            byte_stream out_stream;
            byte_stream_writer writer(out_stream);
            response_packet.write(writer);

            networkLayer.send_to(endpoint, out_stream);
            debug::warn("Incorrect packet version. Sending back disconnect packet.");
            return;
        }

        Connection clientConnection;

        const bool bNewClient = !clients.HasValidClient(endpoint);

        // Not a new client, drop the packet
        if (!bNewClient)
        {
            return;
        }

        debug::info("New client connecting.");

        ClientConnection& client = clients.AddClient(endpoint);
        client.ReceiveServerQueryPacket(); 
    }
};
