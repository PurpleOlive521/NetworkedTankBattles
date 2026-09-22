#pragma once

#include "controller.h"

struct InterpolationState
{
	InterpolationState() = default;
	InterpolationState(double inReceiveTime, Vector2 inPosition, float inRotation);

	double receivedTime = 0.0;
	Vector2 position = {};
	float rotation = 0.0f;
};

class EntityController : public Controller
{
public:

	EntityController() = default;
	virtual ~EntityController() = default;

	// --- Object Lifecycle
	void BeginPlay() override;

	void Update(float DeltaTime) override;

	void OnDestroy() override;

	virtual void OnReplication(const meteor::payload_packet& payload) override;

	virtual bool Write(meteor::byte_stream_writer& writer) override;

	virtual bool Read(meteor::byte_stream_reader& reader) override;

	virtual GameObjectTypes GetType() const override;

	void RecordState(double receiveTime, Vector2 position, float rotation);
	 
protected:
	Vector2 interpolationStart = {};

	std::vector<InterpolationState> stateHistory;

	bool bShouldInterpolate = true;
}; 