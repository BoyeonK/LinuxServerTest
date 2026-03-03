#include <iostream>
#include "SocketWrapper.h"
#include "PacketHandler.h"
#include "IoUringWrapper.h"
#include "ObjectPool.h"
#include "DedicateProcess/DediManager.h"
#include "IPCProtocol/IPCProtocol.pb.h"

void IPCListenSocketWrapper::Init() {
    _listenFd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (_listenFd == -1) throw std::runtime_error("Socket creation failed");

    unlink(_sockPath.c_str());

    struct sockaddr_un addr = {};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, _sockPath.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(_listenFd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(_listenFd);
        throw std::runtime_error("Bind failed for path: " + _sockPath);
    }

    if (listen(_listenFd, _queueSize) == -1) {
        close(_listenFd);
        throw std::runtime_error("Listen failed");
    }
}

Session::Session(int fd, IoUringWrapper* uring) : _fd(fd), _uring(uring) {

}

Session::~Session() {
    if (_fd != -1)
        ::close(_fd);
}

HttpIPCSession::HttpIPCSession(int fd, IoUringWrapper* uring) : Session(fd, uring) {

}

HttpIPCSession::~HttpIPCSession() {

}

void HttpIPCSession::Recv() {
    H2SReadTask* readTask = ObjectPool<H2SReadTask>::Acquire(_fd, _recvBuffer.ReadPos(), _recvBuffer.FreeSize(), this);
    _uring->RegisterRecv(_fd, _recvBuffer.ReadPos(), _recvBuffer.FreeSize(), readTask);
}

void HttpIPCSession::Send(SendBuffer* sendBuffer) {

}

void HttpIPCSession::OnReadComplete(int readBytes) {
    //IOCP때랑은 다르게, 이 함수가 불릴 시점에는 이미 커널이 받아 왔음. 여기서 들어올 수 있는지에 대한 유효성 검사 X
    //대신, 유효한 패킷인지에 대한 검사를 여기서 해 주어야 할 듯?
    _recvBuffer.OnRead(readBytes);
    if (readBytes > 0) {
        //TODO : OnProcess시점부터의 패킷 처리 시작.
        while (true) {
            size_t currentDataSize = _recvBuffer.DataSize();

            if (currentDataSize < sizeof(PacketHeader)) {
                break;
            }
            PacketHeader header = *(reinterpret_cast<PacketHeader*>(_recvBuffer.ProcessedPos()));

            if (currentDataSize < header._size) {
                break;
            }

            if (PacketHandler::HandlePacket(this, _recvBuffer.ProcessedPos(), header._size)){
                _recvBuffer.OnProcess(header._size);
            } else {
                //TODO : 뭔가 잘못됨 뭔가 뭔가임
                break;
            }
        }
        Recv();
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

void HttpIPCSession::OnWriteComplete(int result) {

}

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