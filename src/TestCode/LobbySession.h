#pragma once
#include <memory>
#include <string>
#include <vector>
#include <atomic>

class LobbyServerService;

class LobbySession : public std::enable_shared_from_this<LobbySession> {
public:
    //TODO : 최초에 ServiceFactory함수에서 특정 생성자로 생성하기
    LobbySession() {};
    LobbySession(int sockfd, LobbyServerService* owner);
    virtual ~LobbySession();

    //TODO : 일단 Connect까지 진행한 후, 진행되는지 확인하기.
    void ProcessConnect();
    //void Send(const std::vector<char>& buffer);
    void Disconnect();

    //virtual void OnRecv(const char* buffer, int len);
    //virtual void OnSend(int len);
    virtual void OnDisconnected();

public:
    int GetSocketFD() const { return _sockfd; }
    int GetSessionID() const { return _sessionID; }
    void SetSocketFD(int socketFD) { _sockfd = socketFD; }
    void SetSessionId(int sessionId) { _sessionID = sessionId; }
    void SetService(LobbyServerService* pLSService) { _ownerService = pLSService; }
    bool IsConnected() const { return _isConnected; }

    int _sockfd = -1;
    int _sessionID = -1;
    std::atomic<bool> _isConnected = { false };

    LobbyServerService* _ownerService = nullptr;
    // char _recvBuffer[4096];
};