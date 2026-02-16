#pragma once
#include <liburing.h>
#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory>

#include "IOTask.h"
#include "SendBuffer.h"

class IoUringWrapper {
public:
    enum {
        SEND_BUFFER_CHUNK_SIZE = 65536,
    };
    IoUringWrapper();
    ~IoUringWrapper();

    IoUringWrapper(const IoUringWrapper&) = delete;
    IoUringWrapper& operator=(const IoUringWrapper&) = delete;

    void ExecuteCQTask();
    void RegisterAcceptTask(int listenFd, IOTask* task);

    SendBuffer* OpenSendBuffer(uint32_t allocSize);

private:
    struct io_uring _ring;
    std::shared_ptr<SendBufferChunk> _sendBufferChunkRef = nullptr;
};

extern IoUringWrapper* IORing;