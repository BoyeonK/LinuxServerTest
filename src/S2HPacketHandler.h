#pragma once
#include "IPCProtocol/IPCProtocol.pb.h"
#include "SendBuffer.h"

enum : uint16_t {
	PID_MAIN_WELCOME = 0,
	PID_HTTP_WELCOME = 1,
};

struct PacketHeader {
	uint16_t _size;
	uint16_t _id;
};

bool Handle_Invalid(unsigned char* buffer, int32_t len);
//bool Handle_Http_Welcome(IPC_Protocol::HttpWelcome& pkt);

class S2HPacketHandler {
public:
	static SendBuffer* MakeSendBuffer(const IPC_Protocol::HttpWelcome& pkt) { return MakeSendBuffer(pkt, PID_HTTP_WELCOME); }

private:
	template<typename PBType>
	static SendBuffer* MakeSendBuffer(const PBType& pkt, uint16_t pktId) {
		uint16_t dataSize = static_cast<uint16_t>(pkt.ByteSizeLong());
		uint16_t packetSize = dataSize + sizeof(PacketHeader);

		SendBuffer* sendBuffer = IORing->OpenSendBuffer(packetSize);
		if (sendBuffer == nullptr) {
			//TODO : 에러처리
			return nullptr;
		}
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->_size = packetSize;
		header->_id = pktId;
		pkt.SerializeToArray(&header[1]/*(++header)*/, dataSize);
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};

