#pragma once

#include "network.h"
#include <thread>
#include <chrono>

static void print_error_code()
{
	using namespace meteor;
	auto now = time::get_current_time_ms();
	auto error = network::get_last_error();
	debug::error("%3.2fs - %5d: %s",
		time::elapsed_seconds(now),
		error.code(),
		error.c_str());
}

namespace meteor::sharedConstants
{
	static constexpr uint16 DEFAULT_PORT = 54321;
	static constexpr float TICK_RATE = 1.0f / 30;			// The length of a game tick.
	static constexpr float GAME_UPDATE_RATE = 1.0f / 60;	// Time between each game update.

	static constexpr int CRITICAL_MESSAGE_SEND_COUNT = 10; // How many times do we want to send important messages, when sent in bursts?
	
	static constexpr int SCREEN_WIDTH = 1152;
	static constexpr int SCREEN_HEIGHT = 648;

	static constexpr float SIMULATED_PACKET_LOSS = 0.0f; // 0 to 1 value. Higher value equals higher percentage of packets that will be lost.
}

