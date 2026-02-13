#pragma once
#include <map>
#include <mutex>
#include <memory>
#include <vector>
#include <string>
#include <shared_mutex>
#include "LobbySession.h"
#include "NetAddress.h"
#include "concurrentqueue.h"
#include "MyUtils/ThreadLocalTask.h"

class LobbyServerService {
public:
    static LobbyServerService& Instance() {
        static LobbyServerService instance; // C++11부터 Thread-Safe 보장
        return instance;
    }

    LobbyServerService(const LobbyServerService&) = delete;
    LobbyServerService& operator=(const LobbyServerService&) = delete;

    bool Init(NetAddress address, int maxSessionCount = 1000);

    bool Start();
    void Stop();

    std::shared_ptr<LobbySession> CreateLobbySession(int clientSocket);
    bool AddSession(std::shared_ptr<LobbySession> lSessionRef);
    bool ReleaseSession(std::shared_ptr<LobbySession> lSessionRef);
    bool ReleaseSession(int sessionId);

    NetAddress GetAddress() const { return _myAddress; }

    moodycamel::ConcurrentQueue<MyUtils::TLTask*> LobbyServerTaskQueue;

private:
    LobbyServerService();
    ~LobbyServerService();

private:
    NetAddress _myAddress;
    uint16_t _port = 0;

    std::atomic<bool> _isRunning = false;
    uint32_t _maxSessionCount = 100;

    std::shared_mutex _sessionLock; 
    std::map<int, std::shared_ptr<LobbySession>> _sessions;

    std::atomic<int> _sessionIDGenerator = { 1 };
};