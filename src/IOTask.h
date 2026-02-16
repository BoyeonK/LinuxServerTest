#pragma once

#include <iostream>

class IoUringWrapper;

enum IOTaskType {
    CONNECT_IPC,
    DISCONNECT_IPC,
    ACCEPT_IPC,
    READ_IPC,
    SEND_IPC,
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

class AcceptTask : public IOTask {
public:
    AcceptTask(int listenFd, IOTaskType taskType, IoUringWrapper* uring);
    void callback(int result) override;

private:
    IoUringWrapper* _uring;
};

/*
class ReadTask : public IOTask {
public:
    ReadTask(int fd, void* buf, size_t len, IPCSession* session);
    void callback(int result) override;
    
private:
    IPCSession* _session;
};
*/