#include "SendBuffer.h"
#include "ObjectPool.h"
#include <stdexcept>

void SendBufferChunk::Init() {
	_isOpen = false;
	_usedSize = 0;
}

SendBuffer* SendBufferChunk::Open(uint32_t allocSize) {
	if (allocSize > FreeSize())
		return nullptr;

	_isOpen = true;
	SendBuffer* pBuffer = ObjectPool<SendBuffer>::Acquire();
    pBuffer->Init(shared_from_this(), Index(), allocSize);
	return pBuffer;
}

void SendBufferChunk::Close(uint32_t allocSize) {
	if (_isOpen != true) {
        throw std::runtime_error("SendBufferChunk is not open");
    }
	_isOpen = false;
	_usedSize += allocSize;
}

void SendBuffer::Init(std::shared_ptr<SendBufferChunk> chunkRef, unsigned char* index, uint32_t allocSize) {
	_chunkRef = chunkRef;
	_index = index;
	_allocSize = allocSize;
	_writeSize = 0;
}

void SendBuffer::Close(uint32_t writeSize) {
    if (writeSize > _allocSize){
        throw std::runtime_error("writeSize > _allocSize");
    }

	_writeSize = writeSize;
	_chunkRef->Close(writeSize);
}
