#pragma once

#include "network_layer.h"
#include "application.h"
#include "client.h"
#include "replicationManager_client.h"


class ClientApplication final : public Application
{
public:
	ClientApplication();

	bool Init() override;

	void Update(float deltaTime) override;

	void Render() const override;

	void Exit() override;

	double GetSynchronizationTime() override;

	ClientReplicationManager* GetClientReplicationManager();

	ReplicationManager* GetReplicationManager();

private:
	meteor::Client client;

	ClientReplicationManager replicationManager;
};