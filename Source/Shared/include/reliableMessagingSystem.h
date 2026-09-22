#pragma once
#include "core.h"
#include "network.h"
#include "protocol.h"
#include "reliableMessageFactoryTypes.h"
#include <unordered_map>
#include <map>
#include <string>
#include <bitset>
#include <memory>


// This is a rolling limit, which if changed simply decreases the amount of packets-worth of messages we are able to ack and resend.
constexpr int BUFFER_SIZE = 1024;

// The maximum total size that the messages we include in a given packet can occupy. 
constexpr int MAX_MESSAGE_SUM_SIZE = BUFFER_SIZE / 2;

typedef std::shared_ptr<ReliableMessage> ReliableMessagePtr;

// --- Ack
typedef uint32 AckFieldType;

constexpr size_t ACK_BITSET_SIZE = CHAR_BIT * sizeof(AckFieldType);

typedef std::bitset<ACK_BITSET_SIZE> AckBitset;

struct PacketData
{
	PacketData() = default;

	bool acked = false;
};

// Represents a message either waiting to be sent, or waiting to be acked and removed from the queue.
struct BufferedMessage
{
	BufferedMessage() = delete;
	BufferedMessage(ReliableMessagePtr inMessage);

	ReliableMessagePtr storedMessage = nullptr;

	uint32 messageSize = 0U;
};

struct SentPacketMessages
{
	SentPacketMessages() = default;

	// MessageId of all the messages sent in this packet.
	std::vector<uint32> outgoingMessages;
};

class ReliableMessagingSystem
{	
public:
	ReliableMessagingSystem() = default;

	// Tries to insert as many messages as possible up until MAX_MESSAGE_SUM_SIZE into the given stream.
	void FillPayloadStream(meteor::byte_stream_writer& writer, const meteor::payload_packet& header);

	// Reads all messages until a non-message is found or the stream is empty.
	void ReceiveMessageStream(meteor::byte_stream_reader& reader, const meteor::payload_packet& header);

	// Queues up the message to be sent.
	void SendMessage(ReliableMessagePtr message);

	// Queues up all the message to be sent.
	void SendMessages(std::vector<ReliableMessagePtr> messages);

	// Returns the latest message that we received. Will only return messages in order, so if we are still expecting a message to arrive earlier 
	// in the queue or the queue is empty, nullptr will be returned.
	ReliableMessagePtr FetchLatestMessage();

	// Empties the receive queue.
	void ClearReceivedMessages();

	// Converts the ack bitfield to a displayable series of 1s and 0s.
	std::string GetDebugString() const;

private:
	inline int ToIndex(uint32 sequence) const;

	// Encodes a bitfield containing the status on received packets, relative to the sequence. Bit 'n' corresponds to packet 'sequence - n'.
	AckBitset GetAcknowledgeBitfield(uint32 acknowledge) const;

	const PacketData* GetPacketData(uint32 sequence) const;

	void AcknowledgePacket(uint32 sequence, uint32 acknowledge, AckBitset acknowledgeBits);

	[[nodiscard("PacketData not modified.")]] PacketData& InsertPacketData(uint32 sequence);

	// Local received packet sequence buffer
	uint32 sequenceBuffer[BUFFER_SIZE] = { 0U }; 

	// Each index represents a sent packet and it's ack state.
	PacketData packetData[BUFFER_SIZE] = {};

	// Contains all unacked messages that we are waiting to send, or have confirmed that they have been received
	// Maps a message id to a message.
	std::unordered_map<uint32, BufferedMessage> messagesToSend; 

	// Maps a sequence to a set of messageIds sent in the packet.
	std::unordered_map<uint32, SentPacketMessages> sentMessages; 

	// Maps a messageId to the message itself.
	std::map<uint32, ReliableMessagePtr> receivedMessages;

	uint32 oldestUnackedMessageId = 0U;

	// The next message we are expecting to return to a queurier.
	uint32 nextReceiveMessageId = 0U;
	
	// The id of the next message we will send.
	uint32 nextMessageId = 0U;

	// Cached for debugging purposes & printing only.
	AckBitset latestBitfield;
};

