#include "RecvBuffer.h"

RecvBuffer::RecvBuffer() : _bufferSize(BUFFER_SIZE) {
	_capacity = BUFFER_SIZE * 2;
	_buffer.resize(_capacity);
}

void RecvBuffer::Clean() {
	int32_t remainedBufSize = DataSize();
	if (remainedBufSize == 0) 
		_processedPos = _readPos = 0;
	else {
		if (FreeSize() < _bufferSize) {
			::memcpy(&_buffer[0], &_buffer[_processedPos], remainedBufSize);
            _processedPos = 0;
			_readPos = remainedBufSize;
		}
	}
}

bool RecvBuffer::OnRead(int32_t readSize) {
    //받아 올 크기보다, 남아있는 Recv버퍼의 크기가 작다면 false.
    if (readSize > FreeSize())
		return false;

	//아니면 true를 반환.
	_readPos += readSize;
	return true;
}

bool RecvBuffer::OnProcess(int32_t processedSize) {
    //Process하겠다고 선언한 Buffer의 크기보다 처리가 남은 Buffer의 크기가 작다면 무언가 문제가 있는 상황
	if (processedSize > DataSize())
		return false;

	//그렇지 않은 경우, 모든 버퍼가 넘어왔으며, 처리 될 (혹은 처리 된) 것으로 간주.
	_processedPos += processedSize;
	return true;
}