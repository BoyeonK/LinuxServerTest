#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include "MyUtils/GlobalVariables.h"
#include "MyUtils/Thread.h"
#include "TestCode/LobbyServerService.h"
#include "TestCode/MultiThreadTest.h"

int main() {
    NetAddress myAddress("0.0.0.0", 7777);
    LobbyServerService LSService(myAddress);

    const int WORKER_COUNT = 5;
    for (int i = 0; i < WORKER_COUNT; i++) {
        MyUtils::GThreadManager->Launch([=]() {
            while (true) {
                MyUtils::ThreadManager::GetRegisteredActorAndProcess();
                std::this_thread::yield();
            }
        });
    }
    MyUtils::GThreadManager->Join();

    return 0;
}