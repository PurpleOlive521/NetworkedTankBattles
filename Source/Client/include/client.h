#pragma once

#include "network_layer.h"
#include "constants_client.h"
#include "constants_shared.h"
#include "messages.h"
#include "serverConnection.h"
#include "protocol.h"
#include "timer.h"

class ClientApplication;

namespace meteor
{
    constexpr float READ_PACKET_INTERVAL = 0.5f;

    class Client
    {
    public:
        class Listener
        {
        public:
            virtual void OnConnectedToServer() = 0;

            virtual void OnDisconnectedFromServer(bool bWasTimeout) = 0;

            // Usually a stream of replication messages and gameplay events.
            virtual void OnReceiveServerMessages(const meteor::payload_packet& payload, byte_stream_reader& reader) = 0;
        };

        Client() = default;

        bool Init(ClientApplication* inOwningApplication);

        void Update(float deltaTime);

        void Render() const;

        void Exit();

        void HandleConnect(ip_endpoint& endpoint, byte_stream_reader& reader);

        void HandlePayload(ip_endpoint& endpoint, byte_stream_reader& reader);

        void HandleDisconnect(ip_endpoint& endpoint, byte_stream_reader& reader);
        
        void HandlePing(ip_endpoint& endpoint, byte_stream_reader& reader);

        void ProcessIncomingStream(ip_endpoint& endpoint, byte_stream& stream);

        double GetConnectionTimestamp() const;

        void SetListener(Listener* inListener);

        Listener* GetListener();
    
        ip_endpoint clientEndpoint = { ip_address(), sharedConstants::DEFAULT_PORT};

        ClientApplication* owningApplication = nullptr;

    private:

        network_layer networkLayer;
        ServerConnection connectionToServer; 

        std::vector<ip_address> addresses; 

        ip_endpoint serverEndpoint = { ip_address(), sharedConstants::DEFAULT_PORT};

        Listener* listener = nullptr;
    };

} // !meteor::client