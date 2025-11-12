#pragma once

#include "entity.h"
#include "messages.h"
#include "protocol.h"
#include "inputTypes.h"
#include <map>

class ReplicationManager;

class Controller : public GameObject
{
public:
	Controller() = default;

	// --- Object Lifecycle
	virtual void BeginPlay() override;

	virtual void Update(float DeltaTime) override;

	// Called once per tick. Is not guaranteed to be called every tick in case of lag.
	virtual void OnTick();
	
	// Called immediately after the GameObject is updated through replication.
	virtual void OnReplication(const meteor::payload_packet& payload);

	// Writes the controlled objects state into the writer
	virtual bool Write(meteor::byte_stream_writer& writer);

	// Read states into the controlled object from the reader
	virtual bool Read(meteor::byte_stream_reader& reader);

	virtual GameObjectTypes GetType() const override;

	virtual void OnDestroy() override;

	// Called when the controlled entity is changed.
	virtual void OnControlledEntityChanged();

	void SetControlledEntity(Entity* entity);

	Entity* GetControlledEntity() const;

	void SetNetworkId(uint32 Id);

	uint32 GetNetworkId() const;

	void SetReplicationManager(ReplicationManager* inManager);

	bool bGenerateDeathMessage = true;

protected:

	Entity* controlledEntity = nullptr;

	ReplicationManager* replicationManager = nullptr;

	uint32 networkId = 0;
};