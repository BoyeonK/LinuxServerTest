#include "GlobalVariable.h"
#include "IOTask.h"
#include "IoUringWrapper.h"
#include "ObjectPool.h"

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

    int httpIPCsockFd = result;
    std::cout << "New HTTP IPC Accepted! FD: " << httpIPCsockFd << std::endl;

    //TODO : 이 clientFd로 IPCSession을 만들고, 해당 IPCSession으로 Read요청.
    if (HttpSession == nullptr) {
        HttpSession = new HttpIPCSession(httpIPCsockFd, _uring);
        HttpSession->Recv();
    }

    _uring->RegisterAcceptTask(fd, this);  // 다음 accept
    ObjectPool<H2SAcceptTask>::Release(this);
}


H2SReadTask::H2SReadTask(int fd, void* buf, size_t len, HttpIPCSession* pSession) : _pSession(pSession) {
    this->fd = fd;
    this->type = IOTaskType::READ_IPC_HTTP;
}

void H2SReadTask::callback(int readBytes) {
    _pSession->OnReadComplete(readBytes);
    ObjectPool<H2SReadTask>::Release(this);
}