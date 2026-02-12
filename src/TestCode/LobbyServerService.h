#pragma once
#include <map>
#include <mutex>
#include <memory>
#include <vector>
#include <string>
#include "LobbySession.h"
#include "NetAddress.h"

class LobbyServerService {
public:
    LobbyServerService(NetAddress address, int maxSessionCount = 1000);
    ~LobbyServerService();

    bool Start();
    void Stop();

    std::shared_ptr<LobbySession> CreateLobbySession(int clientSocket);
    bool AddSession(std::shared_ptr<LobbySession> lSessionRef);
    bool ReleaseSession(std::shared_ptr<LobbySession> lSessionRef);
    bool ReleaseSession(int sessionId);
    NetAddress GetAddress() const { return _myAddress; }
    
private:
    void AcceptLoop();

private:
    NetAddress _myAddress;
    uint16_t _port = 0;
    atd::atomic<bool> _isRunning = false;
    int _listenSocket = -1;
    struct io_uring _acceptRing;
    uint32_t _maxSessionCount = 100;

    std::shared_mutex _sessionLock; 
    std::map<int, std::shared_ptr<Session>> _sessions;

    std::atomic<int> _sessionIDGenerator = { 1 };
};