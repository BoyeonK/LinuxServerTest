#include "IPCSocketWrapper.h"

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

IPCSession::IPCSession(int fd, IoUringWrapper* uring) {

}

IPCSession::~IPCSession() {

}

void IPCSession::RegisterRead() {

}

void IPCSession::Send(SendBuffer* sendBuffer) {

}

