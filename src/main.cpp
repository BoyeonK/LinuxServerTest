#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include "TestCode/ChildProcessTest.h"

int main() {
    pid_t pid;
    launchNode(pid);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}