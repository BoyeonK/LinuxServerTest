#include "LobbySession.h"
#include <unistd.h>
#include <iostream>
#include "LobbyServerService.h"

LobbySession::LobbySession(int sockfd, LobbyServerService* owner) : _sockfd(sockfd), _ownerService(owner) {
    _isConnected = false;
}

LobbySession::~LobbySession() {
    Disconnect();
}

void LobbySession::ProcessConnect() {
    bool expected = false;
    if (!_isConnected.compare_exchange_strong(expected, true))
        return;

}

void LobbySession::Disconnect() {
    //중복 해제 방지
    bool expected = true;
    if (!_isConnected.compare_exchange_strong(expected, false))
        return;

    if (_sockfd != -1) {
        ::close(_sockfd);
        _sockfd = -1;
    }

    OnDisconnected();
}

void LobbySession::OnDisconnected() {
    if (_ownerService) {
        try {
            auto self = shared_from_this();
            _ownerService->ReleaseSession(self);
        }
        catch (const std::bad_weak_ptr& e) {

        }
    }
}