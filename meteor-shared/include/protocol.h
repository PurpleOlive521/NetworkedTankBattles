// protocol.hpp

#pragma once

#include "network.h"

namespace meteor
{
    constexpr uint32 PROTOCOL_SIGNATURE =   0xbaadf00d;
    constexpr uint32 PROTOCOL_VERSION =     0x00010000;

   enum class protocol_packet_type : uint8 {
      CONNECT,
      DISCONNECT,
      PAYLOAD,
      PING,
      SERVER_QUERY,
   };

   enum class disconnect_reason_type : uint8
   {
       WRONG_VERSION,
       TIMEOUT,
       SERVER_CLOSE,    // Server closing down
	   QUIT,            // Client wants to disconnect
       CUSTOM_REASON,
   };

   struct connect_packet {
       connect_packet() = default;
       connect_packet(double sendTime);

       bool write(byte_stream_writer& writer);
       bool read(byte_stream_reader& reader);

	   uint8 m_type = (uint8)protocol_packet_type::CONNECT;
       uint32 m_signature = PROTOCOL_SIGNATURE;
       uint32 m_version = PROTOCOL_VERSION;
       double m_send_time = 0; // Time when the packet was sent. Used by client to offset their synchronised time by 1/2 RTT and to convert to server time & tick.

   private:
       template <typename T>
       bool serialize(connect_packet& packet, T& stream);
   };

   struct disconnect_packet {
       disconnect_packet() = default;
       disconnect_packet(disconnect_reason_type reason);

       bool write(byte_stream_writer& writer);
       bool read(byte_stream_reader& reader);

       uint8 m_type = (uint8)protocol_packet_type::DISCONNECT;
       uint8 m_reason = 0;
       char m_message[256] = {};

   private:
        template <typename T>
		bool serialize(disconnect_packet& packet, T& stream);
   };

   struct payload_packet {
       payload_packet() = default;

       bool write(byte_stream_writer& writer);
       bool read(byte_stream_reader& reader);

       uint8 m_type = (uint8)protocol_packet_type::PAYLOAD;
       uint32 m_sequence = 0;
       double m_send_time = 0;
       uint32 m_acknowledge = 0; 
   private:
       template <typename T>
	   bool serialize(payload_packet& packet, T& stream);
   };

   struct ping_packet {
       ping_packet() = default; 

       bool write(byte_stream_writer& writer);
       bool read(byte_stream_reader& reader);

       uint8 m_type = (uint8)protocol_packet_type::PING;
       double m_send_time = 0;
   private: 
       template <typename T>
       bool serialize(ping_packet& packet, T& stream);
   };

   struct server_query_packet {
       server_query_packet() = default;

       bool write(byte_stream_writer& writer);
       bool read(byte_stream_reader& reader);

       uint8 m_type = (uint8)protocol_packet_type::SERVER_QUERY;
       uint32 m_signature = PROTOCOL_SIGNATURE;
       uint32 m_version = PROTOCOL_VERSION;

   private:
       template <typename T>
       bool serialize(server_query_packet& packet, T& stream);
   };
} // !meteor
