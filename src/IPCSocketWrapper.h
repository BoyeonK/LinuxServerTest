#pragma once
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

#include "SendBuffer.h"
#include "IoUringWrapper.h"

class IPCListenSocketWrapper {
public:
    IPCListenSocketWrapper(std::string sockPath, int queueSize) 
        : _sockPath(std::move(sockPath)), _queueSize(queueSize), _listenFd(-1) {}

    ~IPCListenSocketWrapper() {
        if (_listenFd != -1) {
            close(_listenFd);
            unlink(_sockPath.c_str());
        }
    }

    IPCListenSocketWrapper(const IPCListenSocketWrapper&) = delete;
    IPCListenSocketWrapper& operator=(const IPCListenSocketWrapper&) = delete;

    void Init();
    int GetFd() const { return _listenFd; }

private:
    int _listenFd;
    std::string _sockPath;
    int _queueSize;
};

class IPCSession {
public:
    IPCSession(int fd, IoUringWrapper* uring);
    virtual ~IPCSession();

    void RegisterRead();
    void Send(SendBuffer* sendBuffer);

    int GetFd() const { return _fd; }

protected:
    virtual void OnReadComplete(int result) {};
    virtual void OnWriteComplete(int result) {};

    int _fd;
    IoUringWrapper* _uring;
    std::array<uint8_t, 65536> _readBuffer;   // read용 버퍼
};
