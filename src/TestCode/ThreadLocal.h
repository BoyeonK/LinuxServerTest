#pragma once
#include <liburing.h>
#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "NetAddress.h"

class IoUringWrapper {
public:
    IoUringWrapper() {
        int ret = io_uring_queue_init(4096, &_ring, 0);
        if (ret < 0) {
            std::cerr << "링을 만들수가 없엉 : " << ret << std::endl;
            exit(1); // 링 못 만들면 서버 죽어야 함
        }
    }

    ~IoUringWrapper() {
        io_uring_queue_exit(&_ring);
    }

    struct io_uring* GetRing() { return &_ring; }

    IoUringWrapper(const IoUringWrapper&) = delete;
    IoUringWrapper& operator=(const IoUringWrapper&) = delete;

private:
    struct io_uring _ring;
};

class Listener {
public:
    Listener(uint16_t port);
    ~Listener();

    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;

    int GetSocket() const { return _listenSocket; }

private:
    bool Start(uint16_t port);
    void Close();

private:
    int _listenSocket = -1;
};

extern thread_local IoUringWrapper LThreadRing;