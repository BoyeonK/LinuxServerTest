#pragma once
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

class IPCSocketWrapper {
public:
    IPCSocketWrapper(std::string sockPath, int queueSize) 
        : _sockPath(std::move(sockPath)), _queueSize(queueSize), _listenFd(-1) {}

    ~IPCSocketWrapper() = default;

    IPCSocketWrapper(const IPCSocketWrapper&) = delete;
    IPCSocketWrapper& operator=(const IPCSocketWrapper&) = delete;

    void Init() {
        _listenFd = socket(AF_UNIX, SOCK_STREAM, 0);
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

private:
    int _listenFd;
    std::string _sockPath;
    int _queueSize;
};