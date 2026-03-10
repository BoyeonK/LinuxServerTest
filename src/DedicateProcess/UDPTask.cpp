#include "UDPTask.h"
#include "DediSessions.h"

D2CRecvTask::D2CRecvTask(int fd, D2CSession* pSession) : _pSession(pSession) {
    this->fd = fd;
    this->type = IOTaskType::READ_CLIENT;
}

void D2CRecvTask::callback(int readBytes) {
    // 1. 세션의 수신 완료 처리 로직 호출
    if (_pSession) {
        _pSession->OnRecvComplete(readBytes);
    }

    // 2. 일회용 작업이 끝났으므로 Object Pool에 자기 자신을 반환
    // (사용하시는 ObjectPool의 반환 메서드 이름에 맞춰 수정하세요)
    ObjectPool<D2CRecvTask>::Release(this);
}

D2CSendTask::D2CSendTask(int fd, SendBuffer* buffer, const sockaddr_in& destAddr) 
: _pBuffer(buffer), _destAddr(destAddr) {
    this->fd = fd;
    this->type = IOTaskType::SEND_CLIENT;

    _iovec.iov_base = _pBuffer->Buffer();
    _iovec.iov_len = _pBuffer->WriteSize();

    std::memset(&_msgHdr, 0, sizeof(_msgHdr));
    _msgHdr.msg_name = &_destAddr;
    _msgHdr.msg_namelen = sizeof(_destAddr);
    _msgHdr.msg_iov = &_iovec;
    _msgHdr.msg_iovlen = 1;
}

void D2CSendTask::callback(int result) {
    if (result < 0) {
        std::cerr << "[UDP Send Error] result: " << result << std::endl;
    }
    _pSession->OnWriteComplete(result);

    //TODO : SendBuffer*의 처리
    ObjectPool<SendBuffer>::Release(_pBuffer);
    ObjectPool<D2CSendTask>::Release(this);
}