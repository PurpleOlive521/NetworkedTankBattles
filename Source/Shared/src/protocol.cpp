#include "protocol.h"

namespace meteor
{
    connect_packet::connect_packet(double sendTime)
    {
		m_send_time = sendTime;
    }

    bool connect_packet::write(byte_stream_writer& writer)
    {
        return serialize(*this, writer);
    }

    bool connect_packet::read(byte_stream_reader& reader)
    {
        return serialize(*this, reader);
    }

    template <typename T>
    bool connect_packet::serialize(connect_packet& packet, T& stream)
    {
        bool success = true;

        success &= stream.serialize(packet.m_type);
        success &= stream.serialize(packet.m_signature);
        success &= stream.serialize(packet.m_version);
		success &= stream.serialize(packet.m_send_time);

        return success;
    }

    disconnect_packet::disconnect_packet(disconnect_reason_type reason)
    {
		m_reason = (uint8)reason;
    }

    bool disconnect_packet::write(byte_stream_writer& writer)
    {
        return serialize(*this, writer);
    }

    bool disconnect_packet::read(byte_stream_reader& reader)
    {
        return serialize(*this, reader);
    }

    template<typename T>
    bool disconnect_packet::serialize(disconnect_packet& packet, T& stream)
    {
        bool success = true;

        success &= stream.serialize(packet.m_type);
        success &= stream.serialize(packet.m_reason);

        for (size_t i = 0; i < sizeof(packet.m_message); ++i)
        {
			bool result = stream.serialize(packet.m_message[i]);

            if (!result)
            {
                break;
            }
        }

        return success;
    }

    bool payload_packet::write(byte_stream_writer& writer)
    {
        return serialize(*this, writer);
    }

    bool payload_packet::read(byte_stream_reader& reader)
    {
        return serialize(*this, reader);
    }

    template<typename T>
    bool payload_packet::serialize(payload_packet& packet, T& stream)
    {
        bool success = true;

        success &= stream.serialize(packet.m_type);
        success &= stream.serialize(packet.m_sequence);
        success &= stream.serialize(packet.m_acknowledge);
        success &= stream.serialize(packet.m_send_time);

        return success;
    }

    bool ping_packet::write(byte_stream_writer& writer)
    {
        return serialize(*this, writer);
    }

    bool ping_packet::read(byte_stream_reader& reader)
    {
        return serialize(*this, reader);
    }

    template<typename T>
    bool ping_packet::serialize(ping_packet& packet, T& stream)
    {
        bool success = true;

        success &= stream.serialize(packet.m_type);
        success &= stream.serialize(packet.m_send_time);

        return success;
    }

    bool server_query_packet::write(byte_stream_writer& writer)
    {
        return serialize(*this, writer);
    }

    bool server_query_packet::read(byte_stream_reader& reader)
    {
        return serialize(*this, reader);
    }

    template <typename T>
    bool server_query_packet::serialize(server_query_packet& packet, T& stream)
    {
        bool success = true;

        success &= stream.serialize(packet.m_type);
        success &= stream.serialize(packet.m_signature);
        success &= stream.serialize(packet.m_version);

        return success;
    }

} // !meteor
