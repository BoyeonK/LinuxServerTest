#pragma once

#include <cstdint>
#include <functional>

#include "IPCProtocol/IPCProtocol.pb.h"
#include "GlobalVariable.h"
#include "SocketWrapper.h"
#include "IoUringWrapper.h"

extern std::function<bool(Session*, unsigned char*, int32_t)> GProtoPacketHandler[UINT16_MAX];

enum : uint16_t {
	PID_MAIN_WELCOME = 0,
	PID_HTTP_WELCOME = 1,
};

#pragma pack(push, 1)
struct PacketHeader {
	uint16_t _id;
	uint16_t _size;
};
#pragma pack(pop)

bool Handle_Invalid(Session* pSession, unsigned char* buffer, int32_t len);
bool Handle_Http_Welcome(Session* pSession, IPC_Protocol::HttpWelcome& pkt);

class S2HPacketHandler {
public:
	static void Init() {
		for (int i=0; i < UINT16_MAX; i++)
			GProtoPacketHandler[i] = Handle_Invalid;

		GProtoPacketHandler[PID_HTTP_WELCOME] = [](Session* pSession, unsigned char* buffer, int32_t len) { return HandlePacket<IPC_Protocol::HttpWelcome>(Handle_Http_Welcome, pSession, buffer, len); };
	}

	static bool HandlePacket(Session* pSession, unsigned char* buffer, int32_t len) {
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		
		return GProtoPacketHandler[header->_id](pSession, buffer, len);
	}

	static SendBuffer* MakeSendBuffer(const IPC_Protocol::HttpWelcome& pkt) { return MakeSendBuffer(pkt, PID_HTTP_WELCOME); }

public:
	static IPC_Protocol::MainWelcome MakeMainWelcomePkt(int32_t value) {
        IPC_Protocol::MainWelcome pkt;
        pkt.set_echo_message(value);
        return pkt;
    }

private:
	template<typename PBType, typename HandlerFunc>
	static bool HandlePacket(HandlerFunc func, Session* pSession, unsigned char* buffer, int32_t len) {
		PBType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(pSession, pkt);
	}

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

