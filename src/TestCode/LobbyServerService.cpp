#include "LobbyServerService.h"
#include "LobbySession.h"
#include <iostream>

using namespace std;

LobbyServerService::LobbyServerService(NetAddress address, int maxSessionCount)
    : _myAddress(address), _maxSessionCount(maxSessionCount) {
    _port = address.GetPort();
}

LobbyServerService::~LobbyServerService() {
    Stop();
}

bool LobbyServerService::Start() {
    if (_isRunning)
        return false;

    _isRunning = true;

    cout << "[LobbyServerService] Started on Port: " << _port << endl;
    cout << "[LobbyServerService] Max Sessions: " << _maxSessionCount << endl;

    return true;
}

void LobbyServerService::Stop() {
    if (!_isRunning)
        return;

    _isRunning = false;
    
    {
        std::unique_lock<std::shared_mutex> lock(_sessionLock);
        _sessions.clear();
    }

    MyUtils::TLTask* task = nullptr;
    while (LobbyServerTaskQueue.try_dequeue(task)) {
        delete task;
    }

    cout << "[LobbyServerService] Stopped." << endl;
}

std::shared_ptr<LobbySession> LobbyServerService::CreateLobbySession(int clientSocket) {
    //TODO : 최초에 ServiceFactory함수에서 Session을 특정 생성자로 생성하기
    std::shared_ptr<LobbySession> session = std::make_shared<LobbySession>();

    int sessionId = _sessionIDGenerator.fetch_add(1);
    session->SetSessionId(sessionId);
    session->SetSocketFD(clientSocket);
    session->SetService(this);

    // TODO : 네트워크 주소 정보 기입
    /*
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    getpeername(clientSocket, (struct sockaddr*)&clientAddr, &clientLen);
    session->SetNetAddress(NetAddress(clientAddr));
    */
    return session;
}

bool LobbyServerService::AddSession(std::shared_ptr<LobbySession> lSessionRef) {
    if (lSessionRef == nullptr)
        return false;

    std::unique_lock<std::shared_mutex> lock(_sessionLock);

    if (_sessions.size() >= _maxSessionCount) {
        cout << "[LobbyServerService] Max session count reached!" << endl;
        return false;
    }

    // map에 추가 (Key: SessionID, Value: SessionPtr)
    _sessions.insert({ lSessionRef->GetSessionID(), lSessionRef });

    return true;
}

bool LobbyServerService::ReleaseSession(std::shared_ptr<LobbySession> lSessionRef) {
    if (lSessionRef == nullptr)
        return false;

    return ReleaseSession(lSessionRef->GetSessionID());
}

bool LobbyServerService::ReleaseSession(int sessionId) {
    std::unique_lock<std::shared_mutex> lock(_sessionLock);

    auto it = _sessions.find(sessionId);
    if (it == _sessions.end())
        return false;

    _sessions.erase(it);
    return true;
}