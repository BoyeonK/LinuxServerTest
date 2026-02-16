#pragma once
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

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