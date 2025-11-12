#pragma once

#include "application.h"
#include "server.h"
#include "replicationManager_server.h"

class ServerApplication final : public Application
{
public:
	ServerApplication();

	bool Init() override;

	void Update(float deltaTime) override;

	void Render() const override;

	void Exit() override;

	ServerReplicationManager* GetServerReplicationManager();

	GameObject* CreateAndAddObject(GameObjectTypes inType) override;

	ReplicationManager* GetReplicationManager() override;


private:
	meteor::Server server;

	ServerReplicationManager replicationManager;
};