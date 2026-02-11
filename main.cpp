#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include "MyUtils/GlobalVariables.h"
#include "MyUtils/Actor.h"
#include "MyUtils/Thread.h"

using namespace std;
using namespace MyUtils;

class Player : public Actor {
public:
    Player(int id) : _id(id) {}

    void IncreaseCount() {
        _count++;
    }

    int GetCount() const { return _count; }
    int GetID() const { return _id; }

private:
    atomic<int> _id;
    atomic<int> _count = 0;
};

int main() {
    const int WORKER_COUNT = 5;
    const int ACTOR_COUNT = 500;
    const int TASKS_PER_ACTOR = 1000;

    std::atomic<bool> isFinished(false);

    for (int i = 0; i < WORKER_COUNT; i++) {
        GThreadManager->Launch([&isFinished]() {
            while (true) {
                // 리눅스에서도 문제없이 돌아갑니다.
                ThreadManager::GetRegisteredActorAndProcess();
                
                if (isFinished.load()) 
                    break;

                std::this_thread::yield();
            }
        });
    }

    // [수정] wcout -> cout, L"" 제거 (리눅스 호환성)
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
            isFinished = true;
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    GThreadManager->Join();

    return 0;
}