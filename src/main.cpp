#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include "MyUtils/GlobalVariables.h"

#include "MyUtils/Thread.h"
#include "TestCode/MultiThreadTest.h"

int main() {
    const int WORKER_COUNT = 5;
    const int ACTOR_COUNT = 500;
    const int TASKS_PER_ACTOR = 1000;

    for (int i = 0; i < WORKER_COUNT; i++) {
        MyUtils::GThreadManager->Launch([=]() {
            while (true) {
                MyUtils::ThreadManager::GetRegisteredActorAndProcess();
                std::this_thread::yield();
            }
        });
    }

    std::cout << "Test Started with " << ACTOR_COUNT << " actors, each posting " << TASKS_PER_ACTOR << " tasks." << std::endl;

    std::vector<std::shared_ptr<Player>> players(ACTOR_COUNT);
    for (int i = 0; i < ACTOR_COUNT; ++i) {
        std::shared_ptr<Player> playerRef = MyUtils::Memory::ObjectPool<Player>::Acquire(i);
        players[i] = playerRef;
    }

    for (int i = 0; i < TASKS_PER_ACTOR; ++i) {
        for (auto& player : players) {
            player->PostTask(&Player::IncreaseCount);
        }
    }

    std::cout << "[Test] All tasks posted. Waiting for completion..." << std::endl;

    while (true) {
        long long total = 0;
        for (auto& player : players) {
            total += player->GetCount();
        }
        std::cout << "\rTotal Count: " << total << " / " << (ACTOR_COUNT * TASKS_PER_ACTOR) << std::flush;
        
        if (total >= (long long)ACTOR_COUNT * TASKS_PER_ACTOR) {
            std::cout << std::endl << "[Test] All tasks completed." << std::endl;
            players.clear();
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    

    MyUtils::GThreadManager->Join();

    return 0;
}