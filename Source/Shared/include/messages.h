// messages.hpp

#pragma once

#include "raylib.h"
#include "network.h"
#include "inputTypes.h"

namespace meteor
{
    enum class message_type : uint8
    {
        NONE = 0,
        REPLICATION,
        INPUT,
        RELIABLE_MESSAGE,

        ENUM_MAX,
    };

	// Indicates that the next data in the stream is the serialized state of the object that this header refers to.
    struct replication_header
    {
        replication_header() = default;
        replication_header(uint32 objectId, uint8 classId);

        bool write(byte_stream_writer& writer);
        bool read(byte_stream_reader& reader);

        uint8 m_type = (uint8)message_type::REPLICATION;
        int32 m_networkId = 0; // Unique id for the object
	    uint8 m_classId = 0;   // Unique id for the class
    };

    struct input_message
    {
        input_message();
        input_message(uint32 inTick, InputState inButtonStates);

        bool write(byte_stream_writer& writer);
        bool read(byte_stream_reader& reader);

        uint8 m_type = (uint8)message_type::INPUT;
        uint32 m_tick = 0; // The tick that the input was performed in.
        std::pair<uint8, bool> m_inputs[5]; // Pair of button and it's current state.
        int16 m_mouse_x = 0;
        int16 m_mouse_y = 0;
    };

    // Server-sent player state. Serialized as object data after a replication_header.
    struct player_state
    {
        player_state() = default;

        bool write(byte_stream_writer& writer);
        bool read(byte_stream_reader& reader);

        uint32 m_sendTick = 0U; // The tick that the latest received input was sent at.

        Vector2 m_position = {};
        Vector2 m_aim_target = {};
        float m_rotation = 0.0f;
        float m_turret_rotation = 0.0f;
        bool m_is_dead = false;
    };

    struct reliable_message_header
    {
        reliable_message_header() = default;
        reliable_message_header(uint32 acknowledge, uint32 acknowledgebits);

        bool write(byte_stream_writer& writer);
        bool read(byte_stream_reader& reader);

        uint8 m_type = (uint8)message_type::RELIABLE_MESSAGE;
        uint32 m_acknowledge = 0U;
        uint32 m_acknowledge_bits = 0U;
    };

} // !meteor

