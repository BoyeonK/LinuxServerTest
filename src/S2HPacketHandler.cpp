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
    cout << "받은 echo_message : " << pkt.echo_message() << endl;

    return true;
}