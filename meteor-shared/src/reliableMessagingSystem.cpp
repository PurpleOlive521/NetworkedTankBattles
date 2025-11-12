#include "reliableMessagingSystem.h"
#include "messages.h"
#include "reliableMessageFactory.h"
#include "replicationManager.h"


BufferedMessage::BufferedMessage(ReliableMessagePtr inMessage)
{
	meteor::byte_stream_measurer measurer;
	messageSize = inMessage->Measure(measurer);

	assert(inMessage);

	storedMessage.swap(inMessage);
}

void ReliableMessagingSystem::FillPayloadStream(meteor::byte_stream_writer& writer, const meteor::payload_packet& header)
{
	AckBitset ackBitset = GetAcknowledgeBitfield(header.m_acknowledge);
	uint32 acknowledgeBits = ackBitset.to_ulong();
	latestBitfield = acknowledgeBits;

	meteor::reliable_message_header messageHeader = { header.m_acknowledge, acknowledgeBits };
	if (!messageHeader.write(writer))
	{
		debug::warn("Stream already full, no space for messages.");
		return;
	}

	SentPacketMessages outgoingPacket = {};
	uint32 messagesTotalSize = 0U;

	// Include all unacked messages from oldest to newest (ascending Id)
	for (uint32 i = oldestUnackedMessageId; i < nextMessageId; i++)
	{
		auto it = messagesToSend.find(i);
		if ( it == messagesToSend.end())
		{
			continue;
		}

		BufferedMessage& message = it->second;
		const bool wouldGoAboveLimit = messagesTotalSize + message.messageSize > MAX_MESSAGE_SUM_SIZE;
		if (!writer.can_fit(message.messageSize) || wouldGoAboveLimit)
		{
			debug::warn("Message could not fit in PayloadStream. Exiting FillPayloadStream.");
			break;
		}

		message.storedMessage->AssignId(i);
		if (!message.storedMessage->Write(writer))
		{
			debug::error("Message could not be written to PayloadStream!");
			return;
		}

		messagesTotalSize += message.messageSize;
		outgoingPacket.outgoingMessages.push_back(i);
	}

	sentMessages.insert({ header.m_sequence, outgoingPacket });
	InsertPacketData(header.m_sequence).acked = false;
}

void ReliableMessagingSystem::ReceiveMessageStream(meteor::byte_stream_reader& reader, const meteor::payload_packet& header)
{
	meteor::reliable_message_header messageHeader;
	
	if (!messageHeader.read(reader))
	{
		debug::warn("No message header found in stream!");
		return;
	}

	// Read and queue up all received messages
	while (reader.has_data())
	{
		ReliableMessageType type = (ReliableMessageType)reader.peek();

		if (type > ReliableMessageType::ENUM_MAX)
		{
			debug::warn("Invalid message type received. Terminating read!");
			break;
		}

		ReliableMessagePtr createdMessage = ReliableMessageFactory::CreateMessage(type);
		
		if (!createdMessage->Read(reader))
		{
			debug::warn("Message could not be read!");
			break;
		}

		// Message is older than the latest received & acked message, no need to put it back in the queue.
		if (createdMessage->id < nextReceiveMessageId)
		{
			continue;
		}

		receivedMessages.emplace(std::pair<uint32, ReliableMessagePtr>(createdMessage->id, std::move(createdMessage)));
	}

	AcknowledgePacket(header.m_sequence, messageHeader.m_acknowledge, messageHeader.m_acknowledge_bits);
}

void ReliableMessagingSystem::SendMessage(ReliableMessagePtr message)
{
	BufferedMessage outMessage = { message };
	messagesToSend.emplace(std::pair<uint32, BufferedMessage>(nextMessageId, outMessage));
	
	nextMessageId++;
}

void ReliableMessagingSystem::SendMessages(std::vector<ReliableMessagePtr> messages)
{
	for (auto& message : messages)
	{
		SendMessage(message);
	}
}

// Ordered as ack-3..ack-2..ack-1..ack in the vector
AckBitset ReliableMessagingSystem::GetAcknowledgeBitfield(uint32 acknowledge) const
{
	AckBitset bitfield;

	for (int i = 0; i < bitfield.size(); i++)
	{
		int bitOffset = acknowledge + i - (int)bitfield.size();

		if (bitOffset < 0)
		{
			bitOffset += BUFFER_SIZE;
		}
		
		const uint32 index = ToIndex(bitOffset);

		if (sequenceBuffer[index] == (uint32)bitOffset)
		{
			bitfield.set(i);
		}
	}

	return bitfield;
}

ReliableMessagePtr ReliableMessagingSystem::FetchLatestMessage()
{
	if (auto message = receivedMessages.find(nextReceiveMessageId); message != receivedMessages.end())
	{
		ReliableMessagePtr out = message->second;
		receivedMessages.erase(message);

		nextReceiveMessageId++;

		return out;
	}

	return nullptr;
}

void ReliableMessagingSystem::ClearReceivedMessages()
{
	receivedMessages.clear();
}

std::string ReliableMessagingSystem::GetDebugString() const
{
	std::string outString;

	outString.resize(ACK_BITSET_SIZE, 'O');

	for (int i = 0; i < ACK_BITSET_SIZE; i++)
	{
		const bool isAcknowledged = latestBitfield.test(i);

		if (isAcknowledged)
		{
			outString[i] = 'X';
		}
	}

	return outString;
}

int ReliableMessagingSystem::ToIndex(uint32 sequence) const
{
	return sequence % BUFFER_SIZE;
}

const PacketData* ReliableMessagingSystem::GetPacketData(uint32 sequence) const
{
	const int index = ToIndex(sequence);
	if (sequenceBuffer[index] == sequence)
	{
		return &packetData[index];
	}
	else
	{
		return nullptr;
	}
}

void ReliableMessagingSystem::AcknowledgePacket(uint32 sequence, uint32 acknowledge, AckBitset acknowledgeBits)
{
	std::vector<uint32> packetsToAck = { sequence };

	// Acknowledge the incoming packet
	uint32 index = ToIndex(sequence);
	sequenceBuffer[index] = sequence;

	// Acknowledge our packets based on the encoded bits
	for (int i = 0; i < acknowledgeBits.size(); i++)
	{
		// Not received, skip
		if(!acknowledgeBits.test(i))
		{
			continue;
		}

		int32 bitOffset = acknowledge - ACK_BITSET_SIZE + i;

		// The corresponding acknowledge is negative
		if (bitOffset < 0)
		{
			continue;
		}

		const uint32 packet = bitOffset;

		packetsToAck.push_back(packet);
	}

	// Iterate over all acked packets in the message and ack them
	for (auto& packetSequence : packetsToAck)
	{
		auto messagesToDequeue = sentMessages.find(packetSequence);

		// No messages associated with packet.
		if (messagesToDequeue == sentMessages.end())
		{
			continue;
		}

		// Iterate over all messages sent in the packet and remove them from the send queue
		for (const auto& message : messagesToDequeue->second.outgoingMessages)
		{
			messagesToSend.erase(message);
		}

		sentMessages.erase(packetSequence);
	}

	// Walk and clean up all new acked messages until either:
	// 1. A unacked message is found in the queue
	// 2. We catch up to the next message id to assign, e.g there is no unacked messages.
	while (auto it = messagesToSend.find(oldestUnackedMessageId) == messagesToSend.end() && oldestUnackedMessageId < nextMessageId)
	{
		oldestUnackedMessageId++;
	}
}

PacketData& ReliableMessagingSystem::InsertPacketData(uint32 sequence)
{
	const int index = ToIndex(sequence);
	sequenceBuffer[index] = sequence;
	return packetData[index];
}
