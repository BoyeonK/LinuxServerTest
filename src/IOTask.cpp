#include "IOTask.h"
#include "IoUringWrapper.h"

AcceptTask::AcceptTask(int listenFd, IOTaskType taskType, IoUringWrapper* uring)
    : _uring(uring) {
    fd = listenFd;
    type = taskType;
}

void AcceptTask::callback(int result) {
    if (result < 0) {
        std::cerr << "Accept failed: " << result << std::endl;
        _uring->RegisterAcceptTask(fd, this);
        return;
    }

    int clientFd = result;
    std::cout << "New HTTP IPC Accepted! FD: " << clientFd << std::endl;


    _uring->RegisterAcceptTask(fd, this);  // 다음 accept
}