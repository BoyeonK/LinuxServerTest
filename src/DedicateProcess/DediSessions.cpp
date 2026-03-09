#include "DediSessions.h"
#include <iostream>
#include "../ObjectPool.h"
#include "../PacketHandler.h"
#include "DediManager.h"

DediIPCSession::~DediIPCSession() {}

void DediIPCSession::BindSocket(int fd) {
    _fd = fd;
    _state = DediIPCSession::SessionState::Ready;
    Recv();
}

void DediIPCSession::Recv() {

}

void DediIPCSession::Send(SendBuffer* sendBuffer) {

}

void DediIPCSession::OnReadComplete(int readBytes) {

}

void DediIPCSession::OnWriteComplete(int result) {

}

void DediTempSession::Recv() {
    DediRecvTask* readTask = ObjectPool<DediRecvTask>::Acquire(_fd, _recvBuffer.ReadPos(), _recvBuffer.FreeSize(), this);
    _uring->RegisterRecv(_fd, _recvBuffer.ReadPos(), _recvBuffer.FreeSize(), readTask);
}

void DediTempSession::OnReadComplete(int readBytes) {
    _recvBuffer.OnRead(readBytes);
    if (readBytes > 0) {
        if (_recvBuffer.DataSize() < sizeof(PacketHeader)) {
            Recv();
            return;
        }

        PacketHeader header = *(reinterpret_cast<PacketHeader*>(_recvBuffer.ProcessedPos()));
        
        if (_recvBuffer.DataSize() < header._size) {
            Recv();
            return;
        }

        void* payloadPtr = _recvBuffer.ProcessedPos() + sizeof(PacketHeader);
        size_t payloadSize = header._size - sizeof(PacketHeader);
        IPC_Protocol::D2MInitComplete pkt;
        if (pkt.ParseFromArray(payloadPtr, static_cast<int>(payloadSize))) {
            int childPid = pkt.pid();
            if (pDediManager->FinalizeConnection(childPid, this->GetFd())) {
                this->ReleaseFd(); // 성공 시 FD 소유권 포기 (DediIPCSession이 가져감)
            } else {
                std::cerr << "[DediTemp] Failed to finalize connection for PID: " << childPid << std::endl;
            }
            
            delete this; 
            return;
        } else {
            // 패킷 해석 실패 (잘못된 접근)
            std::cerr << "[DediTemp] Protobuf Parse Error!" << std::endl;
            delete this;
            return;
        }
    }
    else if (readBytes == 0) {
        //TODO : 0byte Recv 처리
    } 
    else {
        //TODO : 에러 처리
        /*
        switch(readBytes):
        case -EAGAIN:
            break;
        case -EWOULDBLOCK:
            break;
        case -ECONNRESET:
            break;
        */
    }
}

void MainIPCSession::Recv() {

}

void MainIPCSession::Send(SendBuffer* sendBuffer) {
    IPCSendTask* pTask = ObjectPool<IPCSendTask>::Acquire(sendBuffer, this);
    IORing->RegisterIPCSendTask(pTask);
}

void MainIPCSession::OnReadComplete(int readBytes) {

}

void MainIPCSession::OnWriteComplete(int result) {

}