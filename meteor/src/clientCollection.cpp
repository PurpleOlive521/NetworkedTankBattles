#include "clientCollection.h"

namespace meteor
{
	ClientCollection::ClientCollection(network_layer* inNetworkLayer, Server* inOwningServer)
	{
		assert(inNetworkLayer != nullptr);
		networkLayer = inNetworkLayer;

		assert(inOwningServer);
		owningServer = inOwningServer;
	}

	void ClientCollection::GetOrAddClient(const ip_endpoint& endpoint, ClientConnection& outConnection)
	{
		for (auto it = connections.begin(); it != connections.end(); ++it)
		{
			if (it->IsActive() && it->endpoint == endpoint)
			{
				outConnection = *it;
				return;
			}
		}

		outConnection = AddClient(endpoint);
	}

	ClientConnection& ClientCollection::AddClient(const ip_endpoint& endpoint)
	{
		ClientConnection newConnection(endpoint, networkLayer, owningServer);
		return connections.emplace_back(newConnection);
	}

	bool ClientCollection::HasClient(ip_endpoint endpoint) const
	{
		for (auto it = connections.begin(); it != connections.end(); ++it)
		{
			if (it->endpoint == endpoint)
			{
				return true;
			}
		}

		return false;
	}

	bool ClientCollection::HasValidClient(ip_endpoint endpoint) const
	{
		for (auto it = connections.begin(); it != connections.end(); ++it)
		{
			if (it->endpoint == endpoint && it->IsActive())
			{
				return true;
			}
		}

		return false;
	}

	ClientConnection& ClientCollection::GetValidClient(ip_endpoint endpoint)
	{
		for (auto it = connections.begin(); it != connections.end(); ++it)
		{
			if (it->IsActive() && it->endpoint == endpoint)
			{
				return *it;
			}
		}

		debug::error("Tried to get a client that doesn't exist or is disconnected!");
		return connections[0];
	}

	int ClientCollection::GetClientCount() const
	{
		return (int)connections.size();
	}

	int ClientCollection::GetValidClientCount() const
	{
		int count = 0;
		for (auto it = connections.begin(); it != connections.end(); ++it)
		{
			if (it->IsActive())
			{
				count++;
			}
		}

		return count;
	}

	void ClientCollection::UpdateCollection()
	{
		for (auto it = connections.begin(); it != connections.end(); ++it)
		{
			if(it->IsActive() && it->IsTimedOut())
			{
				it->Timeout();
			}
		}
	}

	bool ClientCollection::SendToValidClients(const byte_stream& stream, int sendCount)
	{
		bool bSuccess = true;

		for (auto it = connections.begin(); it != connections.end(); ++it)
		{
			if(it->IsActive())
			{
				for(int i = 0; i < sendCount; i++)
				{
					if(!it->SendPacketTo(it->endpoint, stream))
					{
						bSuccess &= false;
					}
				}
			}
		}

		return bSuccess;
	}
}
