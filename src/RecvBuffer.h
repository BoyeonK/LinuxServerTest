#pragma once

#include <vector>
#include <cstdint>
#include <cstring>

class RecvBuffer {
    enum {
        BUFFER_SIZE = 32768
    };

public:
    RecvBuffer();
    ~RecvBuffer() {}

    void Clean();
    bool OnRead(int32_t readSize);
    bool OnProcess(int32_t processedSize);
    
    unsigned char* ReadPos() { return &_buffer[_readPos]; }
    unsigned char* ProcessedPos() { return &_buffer[_processedPos]; }
    int32_t DataSize() { return _readPos - _processedPos; }
    int32_t FreeSize() { return _capacity - _readPos; }

private:
    uint32_t _readPos = 0;
    uint32_t _processedPos = 0;
    uint32_t _bufferSize = 0;
    uint32_t _capacity = 0;
    std::vector<unsigned char> _buffer;
};