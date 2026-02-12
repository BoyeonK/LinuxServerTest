#include <atomic>
#include "MyUtils/Actor.h"

class Player : public MyUtils::Actor {
public:
    Player(int id) : _id(id) {}

    void IncreaseCount() {
        _count++;
    }

    int GetCount() const { return _count; }
    int GetID() const { return _id; }

private:
    std::atomic<int> _id;
    std::atomic<int> _count = 0;
};