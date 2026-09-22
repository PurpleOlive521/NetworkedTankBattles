#pragma once

#include "network.h"
#include "clientConnection.h"

class Server;

namespace meteor
{
	class ClientCollection
	{
	public:

		ClientCollection() = default;
		ClientCollection(network_layer* inNetworkLayer, Server* inOwningServer);

		void GetOrAddClient(const ip_endpoint& endpoint, ClientConnection& outConnection);

		ClientConnection& AddClient(const ip_endpoint& endpoint);
		
		// Returns true if the client has ever connected to us, false otherwise.
		bool HasClient(ip_endpoint endpoint) const;
		
		// Returns true if the client is currently connected and active, false otherwise.
		bool HasValidClient(ip_endpoint endpoint) const;

		ClientConnection& GetValidClient(ip_endpoint endpoint);

		int GetClientCount() const;

		int GetValidClientCount() const;

		void UpdateCollection();

		// Returns false if it failed to send to any client. Sends to all valid clients, sendCount amount of times.
		// Use with care since it doesn't update the state when sending!
		bool SendToValidClients(const byte_stream& stream, int sendCount = 1);

		// Do not modify directly!
		std::vector<ClientConnection> connections;

	private:
		network_layer* networkLayer = nullptr;

		Server* owningServer = nullptr;
	};

}