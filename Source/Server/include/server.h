#pragma once

#include "network.h"
#include "messages.h"
#include "network_layer.h"
#include "protocol.h"
#include "clientCollection.h"
#include "timer.h"

class ServerApplication;

namespace meteor
{
    class Server
    {
    public:

        class Listener
        {
        public:
            // Returns the networkId of the PlayerController created for this client.
			virtual uint32 OnClientConnected(uint32 clientId) = 0;

			virtual void OnClientDisconnected(uint32 clientId, bool bWasTimeout) = 0;

            virtual void OnReceiveClientMessages(const meteor::payload_packet& payload, byte_stream_reader& reader, uint32 clientId) = 0;
        };

        Server() = default;

        bool Init(ServerApplication* inOwningApplication);

        void Update(float DeltaTime);

        void Render() const;

        void Exit();

		void SetListener(Listener* inListener);

        Listener* GetListener();

    private:
        void HandleConnect(ip_endpoint& endpoint, byte_stream_reader& reader);

        void HandlePayload(ip_endpoint& endpoint, byte_stream_reader& reader);

        void HandleDisconnect(ip_endpoint& endpoint, byte_stream_reader& reader);

        void HandlePing(ip_endpoint& endpoint, byte_stream_reader& reader);
        
        void HandleServerQuery(ip_endpoint& endpoint, byte_stream_reader& reader);

        void ProcessIncomingStream(ip_endpoint& endpoint, byte_stream& stream);

        network_layer networkLayer;

        ClientCollection clients; 

        Timer sendPacketTimer;

        ServerApplication* owningApplication = nullptr;

        Listener* listener = nullptr;

        std::vector<ip_address> addresses;
    };
    
} // !meteor