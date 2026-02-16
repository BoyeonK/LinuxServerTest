#pragma once

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
    AcceptTask(int listenFd, IOTaskType taskType) {
        this->fd = listenFd;
        this->type = taskType;
    }

    void callback(int result) override {
        if (result < 0) {
            std::cerr << "Accept failed: " << result << std::endl;
            return;
        }

        int clientFd = result;
        std::cout << "New IPC Accepted! FD: " << clientFd << std::endl;
    }
};