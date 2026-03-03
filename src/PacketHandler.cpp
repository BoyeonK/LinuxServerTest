#include "PacketHandler.h"
#include <iostream>

using namespace std;

std::function<bool(Session*, unsigned char*, int32_t)> GProtoPacketHandler[UINT16_MAX];

bool Handle_Invalid(Session* pSession, unsigned char* buffer, int32_t len) {
#ifdef _DEBUG
    cout << "Something goes wrong, Client sent invalid packet" << endl;
#endif
    return false;
}

bool Handle_H2M_Welcome(Session* pSession, IPC_Protocol::H2MWelcome& pkt) {
    cout << "H3-2 - OK 받은 echo_message : " << pkt.echo_message() << endl;

    return true;
}

bool Handle_H2M_MatchMake(Session* pSession, IPC_Protocol::H2MMatchMake& pkt) {
    cout << "매치 요청 테스트용 콘솔 출력" << endl;

    return true;
}

bool Handle_H2M_MatchMakeCancel(Session* pSession, IPC_Protocol::H2MMatchMakeCancel& pkt) {
    cout << "매치취소 요청 테스트용 콘솔 출력" << endl;

    return true;
}

bool Handle_D2M_InitComplete(Session* pSession, IPC_Protocol::D2MInitComplete& pkt) {
    cout << "매치취소 요청 테스트용 콘솔 출력" << endl;

    return true;
}