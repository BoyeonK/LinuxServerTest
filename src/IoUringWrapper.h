#pragma once
#include <liburing.h>
#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "IOTask.h"

class IoUringWrapper {
public:
    IoUringWrapper();
    ~IoUringWrapper();

    IoUringWrapper(const IoUringWrapper&) = delete;
    IoUringWrapper& operator=(const IoUringWrapper&) = delete;

    void ExecuteCQTask();
    void RegisterAcceptTask(int listenFd, IOTask* task);

private:
    struct io_uring _ring;
};