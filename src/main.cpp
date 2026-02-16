#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include "IoUringWrapper.h"
#include "HTTPserver.h"
#include "IPCSocket.h"
#include "ObjectPool.h"

int main() {
    IoUringWrapper IUwrapper;

    try {
        IPCSocketWrapper httpsIpc("/tmp/https.sock", 1);
        httpsIpc.Init();

        IPCSocketWrapper dedicateIpc("/tmp/dedicate.sock", 128);
        dedicateIpc.Init();

        pid_t nodePid;
        if (launchNode(nodePid))
            std::cout << "Node.js server launched (PID: " << nodePid << ")" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "소켓 생성 실패 : " << e.what() << std::endl;
        return 1;
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}