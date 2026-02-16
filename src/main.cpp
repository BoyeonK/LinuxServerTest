#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include "TestCode/ThreadLocal.h"
#include "IPCSocket.h"

int main() {
    try {
        IPCSocketWrapper httpsIpc("/tmp/https.sock", 1);
        httpsIpc.Init();

        IPCSocketWrapper dedicateIpc("/tmp/dedicate.sock", 128);
        dedicateIpc.Init();
    } catch (const std::exception& e) {
        std::cerr << "소켓 생성 실패 : " << e.what() << std::endl;
        return 1;
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}