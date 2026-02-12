#pragma once
#include <memory>
#include <string>
#include <vector>
#include <atomic>

class LobbyServerService;

class LobbySession : public std::enable_shared_from_this<LobbySession> {
public:
    LobbySession(int sockfd, LobbyServerService* owner);
    virtual ~LobbySession();

    void ProcessConnect();
    void Send(const std::vector<char>& buffer);
    void Disconnect();

    virtual void OnRecv(const char* buffer, int len);
    virtual void OnSend(int len);
    virtual void OnDisconnected();

public:
    int GetSocketFD() const { return _sockfd; }
    int GetSessionID() const { return _sessionID; }
    bool IsConnected() const { return _isConnected; }

    int _sockfd = -1;
    int _sessionID = -1;
    std::atomic<bool> _isConnected = { false };

    LobbyServerService* _ownerService = nullptr;
    // char _recvBuffer[4096];
};