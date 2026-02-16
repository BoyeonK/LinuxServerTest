#pragma once

#include "IPCProtocol/IPCProtocol.pb.h"

class S2HPacketMaker {
public:
    static IPC_Protocol::MainWelcome(int32_t value) {
        IPC_Protocol::MainWelcome pkt;
        pkt.set_echo_message(value);
        return pkt;
    }
}