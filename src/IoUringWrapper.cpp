#include "IoUringWrapper.h"
#include "IOTask.h"

IoUringWrapper* IORing = new IoUringWrapper();

IoUringWrapper::IoUringWrapper() {
    int ret = io_uring_queue_init(4096, &_ring, 0);
    if (ret < 0) {
        std::cerr << "링을 만들수가 없엉 : " << ret << std::endl;
        exit(1); // 링 못 만들면 서버 죽어야 함
    }

    //_sendBufferChunkRef = make_shared<SendBufferChunk>();
}

IoUringWrapper::~IoUringWrapper() {
    io_uring_queue_exit(&_ring);
}

void IoUringWrapper::ExecuteCQTask() {
    struct io_uring_cqe* cqe = nullptr;

    while (io_uring_peek_cqe(&_ring, &cqe) == 0) {
        if (!cqe) break;

        IOTask* task = reinterpret_cast<IOTask*>(io_uring_cqe_get_data(cqe));
        if (task) task->callback(cqe->res);

        io_uring_cqe_seen(&_ring, cqe);
    }
}

void IoUringWrapper::RegisterSendTask(int socketFd, IOTask* task) {
    
}

void IoUringWrapper::RegisterAcceptTask(int listenFd, IOTask* task) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(&_ring);
    if (!sqe) return;

    io_uring_prep_accept(sqe, listenFd, nullptr, nullptr, 0);
    io_uring_sqe_set_data(sqe, task);
    
    io_uring_submit(&_ring);
}

SendBuffer* IoUringWrapper::OpenSendBuffer(uint32_t allocSize) {
    if (allocSize > SendBufferChunk::SEND_BUFFER_CHUNK_SIZE) {
        throw std::runtime_error("writeSize > _allocSize");
    }

	if (_sendBufferChunkRef == nullptr or _sendBufferChunkRef->FreeSize() < allocSize) {
		std::shared_ptr<SendBufferChunk> newChunk = std::make_shared<SendBufferChunk>();
		_sendBufferChunkRef = newChunk;
	}

    if (_sendBufferChunkRef->IsOpen() == true) {
        throw std::runtime_error("sendBuffer Error");
    }

	return _sendBufferChunkRef->Open(allocSize);
}