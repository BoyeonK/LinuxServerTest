#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <cstdint>

class SendBuffer;

class SendBufferChunk : public std::enable_shared_from_this<SendBufferChunk> {
public:
    enum {
        SEND_BUFFER_CHUNK_SIZE = 65536,
    };
    SendBufferChunk() { Init(); }

    void Init();
    SendBuffer* Open(uint32_t allocSize);
    void Close(uint32_t allocSize);

    bool IsOpen() { return _isOpen; };
    unsigned char* Index() { return &_buffer[_usedSize]; }

    uint32_t FreeSize() { 
        return static_cast<uint32_t>(_buffer.size() - _usedSize); 
    }

private:
    std::array<unsigned char, SEND_BUFFER_CHUNK_SIZE> _buffer = {};
    bool _isOpen = false;
    uint32_t _usedSize = 0;
};

class SendBuffer {
public:
    SendBuffer() { }
    ~SendBuffer() {
        _chunkRef = nullptr;
    }

    void Init(std::shared_ptr<SendBufferChunk> chunkRef, unsigned char* index, uint32_t allocSize);

    unsigned char* Buffer() { return _index; }
    uint32_t AllocSize() { return _allocSize; }
    uint32_t WriteSize() { return _writeSize; }
    void Close(uint32_t writeSize);

private:
    std::shared_ptr<SendBufferChunk> _chunkRef;
    unsigned char* _index = nullptr;

    uint32_t _allocSize = 0;
    uint32_t _writeSize = 0;
};