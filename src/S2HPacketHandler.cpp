#include "S2HPacketHandler.h"
#include <iostream>

using namespace std;

std::function<bool(Session*, unsigned char*, int32_t)> GProtoPacketHandler[UINT16_MAX];

bool Handle_Invalid(Session* pSession, unsigned char* buffer, int32_t len) {
#ifdef _DEBUG
    cout << "Something goes wrong, Client sent invalid packet" << endl;
#endif
    return false;
}

bool Handle_Http_Welcome(Session* pSession, IPC_Protocol::HttpWelcome& pkt) {
    cout << "H3-2 - OK 받은 echo_message : " << pkt.echo_message() << endl;

    return true;
}

bool Handle_Http_MatchMake(Session* pSession, IPC_Protocol::HttpMatchMake& pkt) {
    cout << "매치 요청 테스트용 콘솔 출력" << endl;

    return true;
}

bool Handle_Http_MatchMakeCancel(Session* pSession, IPC_Protocol::HttpMatchMakeCancel& pkt) {
    cout << "매치취소 요청 테스트용 콘솔 출력" << endl;

    return true;
}