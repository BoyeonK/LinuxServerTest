#include "ThreadLocal.h"

thread_local IoUringWrapper LThreadRing;

Listener::Listener(uint16_t port) {
    if (!Start(port)) {
        std::cerr << "Fatal: Listener Start Failed at port " << port << std::endl;
        exit(1); 
    }
}

Listener::~Listener() {
    Close();
}

bool Listener::Start(uint16_t port) {
    _listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (_listenSocket < 0) return false;

    // SO_REUSEPORT 설정
    int opt = 1;
    if (::setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) return false;
    if (::setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) return false;

    // Bind
    NetAddress addr("0.0.0.0", port);
    auto sockaddr = addr.GetSockAddr();

    if (::bind(_listenSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) return false;
    if (::listen(_listenSocket, SOMAXCONN) < 0) return false;
    return true;
}

void Listener::Close() {
    if (_listenSocket != -1) {
        ::close(_listenSocket);
        _listenSocket = -1;
    }
}