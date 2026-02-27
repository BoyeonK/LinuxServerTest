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
        std::cerr << "C3-3 - X HTTP IPC 연결 실패: " << result << std::endl;
        _uring->RegisterAcceptTask(fd, this);
        return;
    }

    int httpIPCsockFd = result;
    std::cout << "C3-3 - OK HTTP IPC 연결 성공 FD: " << httpIPCsockFd << std::endl;

    if (HttpSession == nullptr) {
        HttpSession = new HttpIPCSession(httpIPCsockFd, _uring);
        HttpSession->Recv();
    }

    //_uring->RegisterAcceptTask(fd, this); H2S 소켓은 유일해야 함. 나중에 문제가 생겼을 때, 재실행 로직은 구현 예정 없음 ㅋ
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