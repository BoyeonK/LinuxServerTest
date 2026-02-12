#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include "MyUtils/GlobalVariables.h"

#include "MyUtils/Thread.h"
#include "TestCode/MultiThreadTest.h"

using namespace std;
using namespace MyUtils;

int main() {
    const int WORKER_COUNT = 5;
    const int ACTOR_COUNT = 500;
    const int TASKS_PER_ACTOR = 1000;

    for (int i = 0; i < WORKER_COUNT; i++) {
        GThreadManager->Launch([=]() {
            while (true) {
                ThreadManager::GetRegisteredActorAndProcess();
                std::this_thread::yield();
            }
        });
    }

    cout << "Test Started with " << ACTOR_COUNT << " actors, each posting " << TASKS_PER_ACTOR << " tasks." << endl;

    vector<shared_ptr<Player>> players(ACTOR_COUNT);
    for (int i = 0; i < ACTOR_COUNT; ++i) {
        shared_ptr<Player> playerRef = MyUtils::Memory::ObjectPool<Player>::Acquire(i);
        players[i] = playerRef;
    }

    for (int i = 0; i < TASKS_PER_ACTOR; ++i) {
        for (auto& player : players) {
            player->PostTask(&Player::IncreaseCount);
        }
    }

    cout << "[Test] All tasks posted. Waiting for completion..." << endl;

    while (true) {
        long long total = 0;
        for (auto& player : players) {
            total += player->GetCount();
        }
        cout << "\rTotal Count: " << total << " / " << (ACTOR_COUNT * TASKS_PER_ACTOR) << flush;
        
        if (total >= (long long)ACTOR_COUNT * TASKS_PER_ACTOR) {
            cout << endl << "[Test] All tasks completed." << endl;
            players.clear();
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    

    GThreadManager->Join();

    return 0;
}