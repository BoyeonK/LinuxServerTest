#include "IOTask.h"
#include "IoUringWrapper.h"

H2SAcceptTask::H2SAcceptTask(int listenFd, IoUringWrapper* uring) : _uring(uring) {
    fd = listenFd;
    type = IOTaskType::ACCEPT_IPC_HTTP;
}

void H2SAcceptTask::callback(int result) {
    if (result < 0) {
        std::cerr << "Accept failed: " << result << std::endl;
        _uring->RegisterAcceptTask(fd, this);
        return;
    }

    int clientFd = result;
    std::cout << "New HTTP IPC Accepted! FD: " << clientFd << std::endl;
    //TODO : 이 clientFd로 IPCSession을 만들고, 해당 IPCSession으로 Read요청.

    _uring->RegisterAcceptTask(fd, this);  // 다음 accept
}

/*
H2SReadTask::H2SReadTask(int fd, void* buf, size_t len, HttpIPCSession* session) : _session(session) {
    this->fd = fd;
    this->type = IOTaskType::READ_IPC_HTTP;


}
*/