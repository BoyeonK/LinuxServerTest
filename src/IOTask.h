#pragma once

#include <iostream>
#include "SocketWrapper.h"

class IoUringWrapper;

enum IOTaskType {
    CONNECT_IPC_HTTP,
    DISCONNECT_IPC_HTTP,
    ACCEPT_IPC_HTTP,
    READ_IPC_HTTP,
    SEND_IPC,
    
    CONNECT_IPC_DEDICATE,
    DISCONNECT_IPC_DEDICATE,
    ACCEPT_IPC_DEDICATE,
    READ_IPC_DEDICATE,

    CONNECT_CLIENT,
    DISCONNECT_CLIENT,
    ACCEPT_CLIENT,
    READ_CLIENT,
    SEND_CLIENT,
};

class IOTask {
public:
    virtual ~IOTask() = default;

    int fd;
    IOTaskType type;

    virtual void callback(int result) = 0;
};

class H2SAcceptTask : public IOTask {
public:
    H2SAcceptTask(int listenFd, IoUringWrapper* uring);
    void callback(int result) override;

private:
    IoUringWrapper* _uring;
};

class H2SReadTask : public IOTask {
public:
    H2SReadTask(int fd, void* buf, size_t len, HttpIPCSession* pSession);
    void callback(int readBytes) override;
    
private:
    HttpIPCSession* _pSession;
};

class DediAcceptTask : public IOTask {
public:
    DediAcceptTask(int listenFd, IoUringWrapper* uring);
    void callback(int result) override;

private:
    IoUringWrapper* _uring;
};

class DediRecvTask : public IOTask {
public:
    DediRecvTask(int fd, void* buf, size_t len, Session* pSession);
    void callback(int readBytes) override;

private:
    Session* _pSession;
};

class IPCSendTask : public IOTask {
public:
    IPCSendTask(SendBuffer* buffer, Session* session);
    void callback(int result) override;
    
    SendBuffer* pBuffer;
    Session* pSession;
};
