#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include "IoUringWrapper.h"
#include "IOTask.h"
#include "HTTPserver.h"
#include "IPCSocketWrapper.h"
#include "ObjectPool.h"

int main() {
    std::unique_ptr<IPCListenSocketWrapper> httpsIpc;
    std::unique_ptr<IPCListenSocketWrapper> dedicateIpc;
    pid_t nodePid;
    try {
        httpsIpc = std::make_unique<IPCListenSocketWrapper>("/tmp/https.sock", 1);
        httpsIpc->Init();

        dedicateIpc = std::make_unique<IPCListenSocketWrapper>("/tmp/dedicate.sock", 128);
        dedicateIpc->Init();

        if (launchNode(nodePid))
            std::cout << "Node.js server launched (PID: " << nodePid << ")" << std::endl;

        H2SAcceptTask* httpAcceptTask = ObjectPool<H2SAcceptTask>::Acquire(
            httpsIpc->GetFd(), 
            IORing
        );
        IORing->RegisterAcceptTask(httpsIpc->GetFd(), httpAcceptTask);

    } catch (const std::exception& e) {
        std::cerr << "소켓 생성 ~ 초기화 실패 : " << e.what() << std::endl;
        _exit(1);
    }

    while (true) {
        IORing->ExecuteCQTask();

        std::this_thread::yield();
    }

    return 0;
}