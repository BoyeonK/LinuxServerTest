#include "DedicateMain.h"
#include <iostream>
// #include "IoUringWrapper.h"
// #include <sys/socket.h>
// #include <sys/un.h>

int DedicateMain(int argc, char* argv[]) {
    std::cout << "[Dedicated] 데디케이티드 전용 프로세스 부팅 완료!" << std::endl;

    try {
        IORing = new IoUringWrapper();
        pRedis = new sw::redis::Redis(redis_url);
    } catch (const std::exception& e) {
        return 1;
    }
    // 2. AF_UNIX 소켓 생성 후 "/tmp/dedicate.sock" 으로 connect() 시도
    // 3. 연결 성공 시 로비 서버에게 "나 PID OOO번 데디 서버인데 준비됐어!" 라고 IPC 메시지 전송
    // 4. io_uring 무한 루프 진입 (CQ 대기)

    while (true) {
        // 데디 전용 이벤트 루프
        // dedi_uring->ExecuteCQTask();
    }

    return 0;
}