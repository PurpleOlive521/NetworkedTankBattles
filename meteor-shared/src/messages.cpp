// messages.cpp

#include "messages.h"
#include "protocol.h"
#include "inputTypes.h"

namespace meteor
{
    replication_header::replication_header(uint32 objectId, uint8 classId) : m_type((uint8)message_type::REPLICATION),
        m_networkId(objectId),
	    m_classId(classId)
    {
    }

    template <typename T>
    bool serialize(replication_header& message, T& stream)
    {
        bool success = true;
        success &= stream.serialize(message.m_type);
        success &= stream.serialize(message.m_networkId);
        success &= stream.serialize(message.m_classId);

        return success;
    }

    bool replication_header::write(byte_stream_writer& writer)
    {
        return serialize(*this, writer);
    }

    bool replication_header::read(byte_stream_reader& reader)
    {
        return serialize(*this, reader);
    }



    template <typename T>
    bool serialize(input_message& message, T& stream)
    {
        bool success = true;
        success &= stream.serialize(message.m_type);
        success &= stream.serialize(message.m_tick);

        success &= stream.serialize(message.m_mouse_x);
        success &= stream.serialize(message.m_mouse_y);

        for (size_t i = 0; i < sizeof(message.m_inputs); ++i)
        {
            success &= stream.serialize(message.m_inputs[i].first);
            success &= stream.serialize(message.m_inputs[i].second);

            if (!success)
            {
                break;
            }
        }

        return success;
    }

    input_message::input_message()
    {
        m_inputs[0] = { (uint8)Button::DOWN, false };
        m_inputs[1] = { (uint8)Button::LEFT, false };
        m_inputs[2] = { (uint8)Button::UP, false };
        m_inputs[3] = { (uint8)Button::RIGHT, false };
    }

    input_message::input_message(uint32 inTick, InputState inButtonStates)
    {
        m_tick = inTick;

        int i = 0;
        for (auto& button : inButtonStates.inputStateMap)
        {
            m_inputs[i] = std::pair<uint8, bool>((uint8)button.first, button.second);

            if (i >= (int)sizeof(m_inputs))
            {
                break;
            }

            i++;
        }

        m_mouse_x = (uint16)inButtonStates.mousePosition.x;
        m_mouse_y = (uint16)inButtonStates.mousePosition.y;
    }



    bool input_message::write(byte_stream_writer& writer)
    {
        return serialize(*this, writer);
    }

    bool input_message::read(byte_stream_reader& reader)
    {
        return serialize(*this, reader);
    }



    template <typename T>
    bool serialize(player_state& message, T& stream)
    {
        bool success = true;

        success &= stream.serialize(message.m_sendTick);

        success &= stream.serialize(message.m_position.x);
        success &= stream.serialize(message.m_position.y);

        success &= stream.serialize(message.m_aim_target.x);
        success &= stream.serialize(message.m_aim_target.y);

        success &= stream.serialize(message.m_rotation);
        success &= stream.serialize(message.m_turret_rotation);

        return success;
    }

    bool player_state::write(byte_stream_writer& writer)
    {
        return serialize(*this, writer);
    }

    bool player_state::read(byte_stream_reader& reader)
    {
        return serialize(*this, reader);
    }




    reliable_message_header::reliable_message_header(uint32 acknowledge, uint32 acknowledgebits)
    {
        m_acknowledge = acknowledge;
        m_acknowledge_bits = acknowledgebits;
    }

    template <typename T>
    bool serialize(reliable_message_header& message, T& stream)
    {
        bool success = true;

        success &= stream.serialize(message.m_type);
        success &= stream.serialize(message.m_acknowledge);
        success &= stream.serialize(message.m_acknowledge_bits);

        return success;
    }

    bool reliable_message_header::write(byte_stream_writer& writer)
    {
        return serialize(*this, writer);
    }

    bool reliable_message_header::read(byte_stream_reader& reader)
    {
        return serialize(*this, reader);
    }

} // !meteor
