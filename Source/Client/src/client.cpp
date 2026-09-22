#include "client.h"
#include "constants_client.h"
#include "application.h"

#include "gameObject.h"

namespace meteor
{
    bool Client::Init(ClientApplication* inOwningApplication)
    {
        owningApplication = inOwningApplication;
        if (!owningApplication)
        {
            debug::error("Invalid owning application!");
            return false;
        }

        if (network::query_local_addresses(addresses))
        {
            clientEndpoint = {addresses.front(), sharedConstants::DEFAULT_PORT};
        }
        
        // Generate port
        clientEndpoint.m_port = (uint16)GetRandomValue(50001, 54000);

        if (!networkLayer.open(clientEndpoint))
        {
            debug::error("Could not open socket at client endpoint!");
            return false;
        }
        
        connectionToServer = ServerConnection(serverEndpoint, &networkLayer, this);
        connectionToServer.ConnectToServer();

        return true;
    }

    void Client::Update(float deltaTime)
    {
        if (connectionToServer.IsTimedOut())
        {
            connectionToServer.Timeout();
        }

        std::function<void(ip_endpoint&, byte_stream&)> callback = std::bind(&Client::ProcessIncomingStream, this, std::placeholders::_1, std::placeholders::_2);
       
        while (networkLayer.receive(callback))
        {

        }
        
        connectionToServer.Update(deltaTime);
    }

    void Client::Render() const
    {
        DrawText(connectionToServer.GetDebugString().c_str(), 10, 10, 24, RAYWHITE);

        if (connectionToServer.status == Connection::Status::DISCONNECTED) {
            int fontSize = 50; 
            int textWidth = MeasureText(connectionToServer.disconnectReason, fontSize);
            Vector2 textPosition = { GetScreenWidth() * 0.5f - textWidth * 0.5f, GetScreenHeight() * 0.5f - fontSize * 0.5f};

            DrawText(connectionToServer.disconnectReason, (int)textPosition.x, (int)textPosition.y, fontSize, RAYWHITE);
        }

        DrawFPS(GetScreenWidth() - 100, 0);
    }

    void Client::Exit()
    {
        // The call doesn't update the connections state, but it doesn't matter since we quit the application after this anyways.
        connectionToServer.SendDisconnectPacket(disconnect_reason_type::QUIT, sharedConstants::CRITICAL_MESSAGE_SEND_COUNT);

        networkLayer.close();
    }

    void Client::HandleConnect(ip_endpoint& endpoint, byte_stream_reader& reader)
    {
        connect_packet packet;
        if (!packet.read(reader))
        {
            debug::warn("Failed to read connect packet!");
            return;
        }

        connectionToServer.ReceiveConnectPacket(packet, endpoint);
    }

    void Client::HandlePayload(ip_endpoint& endpoint, byte_stream_reader& reader)
    {
        payload_packet packet;
        if (!packet.read(reader))
        {
            debug::warn("Failed to read payload packet!");
            return;
        }

        connectionToServer.ReceivePayloadPacket(packet, reader);
    }

    void Client::HandleDisconnect(ip_endpoint & endpoint, byte_stream_reader & reader)
    {
        disconnect_packet packet;
        if (!packet.read(reader))
        {
            debug::warn("Failed to read payload packet!");
            return;
        }

        connectionToServer.ReceiveDisconnectPacket(packet);
    }

    void Client::HandlePing(ip_endpoint& endpoint, byte_stream_reader& reader)
    {
        ping_packet packet; 
        if (!packet.read(reader))
        {
            debug::warn("Failed to read ping packet!");
            return;
        }

        connectionToServer.ReceivePingPacket(packet);
    }

    void Client::ProcessIncomingStream(ip_endpoint& endpoint, byte_stream& stream)
    {
        // Empty packet, discard
        if (!stream.has_data())
        {
            debug::info("Stream has no Data");
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
        }
    }

    double Client::GetConnectionTimestamp() const
    {
        return connectionToServer.connectTimestamp;
    }

    void Client::SetListener(Listener* inListener)
    {
        listener = inListener;
    }

    Client::Listener* Client::GetListener()
    {
        return listener;
    }
}
