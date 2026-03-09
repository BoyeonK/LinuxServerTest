#pragma once
#include "../SocketWrapper.h"
#include "Matchmaker.h"
#include "../IPCProtocol/IPCProtocol.pb.h"
#include "PacketHandler.h"

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
        if (GetAffordablePlayers() >= static_cast<int>(ticketVec.size()) && _state != SessionState::Terminated) {
            _allocatedPlayers += ticketVec.size();

            IPC_Protocol::M2DMakeRoomForThisGroup pkt = MakeM2DMakeRoomForThisGroup(ticketVec);
            _tempMatchPkts.push_back(std::move(pkt));

            if (_state == SessionState::Ready)
                FlushPendingTickets(); 

            return true;
        }
        return false;
    }

    void FlushPendingTickets() {
        if (_tempMatchPkts.empty()) return;

        for (auto& pkt : _tempMatchPkts) {
            SendBuffer* sendBuffer = PacketHandler::MakeSendBuffer(pkt);
            Send(sendBuffer);
        }
        _tempMatchPkts.clear();
    }

private:
    static IPC_Protocol::M2DMakeRoomForThisGroup MakeM2DMakeRoomForThisGroup(TicketVector& group) {
		IPC_Protocol::M2DMakeRoomForThisGroup pkt;

        pkt.mutable_ticket_id()->Reserve(group.size());

        for (auto& ticket : group) {
            if (ticket != nullptr) {
                pkt.add_ticket_id(ticket->ticketId);
            }
        }
        return pkt;
	}

    int _pid;
    int _ingamePlayers = 0;
    int _allocatedPlayers = 0;
    SessionState _state;
    std::vector<IPC_Protocol::M2DMakeRoomForThisGroup> _tempMatchPkts;
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