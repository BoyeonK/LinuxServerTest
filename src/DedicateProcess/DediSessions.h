#pragma once
#include "../SocketWrapper.h"
#include "Matchmaker.h"

// M2D
class DediIPCSession : public Session {
static constexpr int MAX_PLAYER_PER_PROCESS = 50;
public:
    enum class SessionState {
        Initializing,
        Ready,
        Terminated,
    };

    DediIPCSession(int pid, IoUringWrapper* uring) : Session(-1, uring), _pid(pid), _state(SessionState::Initializing) {}
    ~DediIPCSession();

    void BindSocket(int fd);

    void Recv() override;
    void Send(SendBuffer* sendBuffer) override;

    void OnReadComplete(int readBytes) override;
    void OnWriteComplete(int result) override;

    int GetAffordablePlayers() const { 
        return MAX_PLAYER_PER_PROCESS - _allocatedPlayers; 
    }

    bool AllocatePlayers(TicketVector& ticketVec) {
        if (GetAffordablePlayers >= ticketVec.size() && _state != SessionState::Terminated) {
            _tempTV.push_back(std::move(ticketVec));
            return true;
        }
        return false;
    }

private:
    int _pid;
    int _ingamePlayers = 0;
    int _allocatedPlayers = 0;
    SessionState _state;
    std::vector<TicketVector> _tempTV;
};

// M2D
class DediTempSession : public Session {
public:
    DediTempSession(int fd, IoUringWrapper* uring) : Session(fd, uring) {};
    ~DediTempSession() {}

    void ReleaseFd() { _fd = -1; }
    void Recv() override;
    void OnReadComplete(int readBytes) override;

private:
    void Send(SendBuffer* sendBuffer) override {}
    void OnWriteComplete(int result) override {}
};

// D2M
class MainIPCSession : public Session {
public:
    MainIPCSession(int fd, IoUringWrapper* uring) : Session(fd, uring) {};
    ~MainIPCSession() {};

    void Recv() override;
    void Send(SendBuffer* sendBuffer) override;

    void OnReadComplete(int readBytes) override;
    void OnWriteComplete(int result) override;
};