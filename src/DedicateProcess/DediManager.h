#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstdlib> 
#include <unordered_map>

#include "DedicateMain.h" 
#include "../GlobalVariable.h"
#include "../SocketWrapper.h"

class DediManager {
public:
    DediManager() = default;
    ~DediManager();

private:
    void SpawnSingleServer() {
        pid_t pid = fork();

        if (pid < 0) {
            std::cerr << "[DediManager] 프로세스 생성(fork) 실패!" << std::endl;
            return;
        }
        // 이론상 else구문의 pSession의 생성이 execl으로 실행된 DedicateMain의 초기화과정보다 늦으면 에러 발생함.
        // 하지만,
        // DediIPCSession 객체 하나 띄우는게, 새로운 프로세스를 실행하고 환경변수 불러오고 io_uring객체 하나 만들고 redis연결하고 unix domain 소켓 만들고 connect요청한 다음 4byte짜리 헤더 뒤에 DediInitComplete패킷을 protobuf로 직렬화한 payload달고 send 하는거보다 늦을게 분명하기에 그냥 넘어감 ㅅㄱㄹ
        else if (pid == 0) {
            execl("./LinuxServerTest", "./LinuxServerTest", "--dedicated", (char*)nullptr);
            std::cerr << "exec 실패! (바이너리 경로 확인 필요)" << std::endl;
            exit(1);
        } 
        else {
            DediIPCSession* pSession = new DediIPCSession(pid, IORing);
            _dediSessions[pid] = pSession;
            std::cout << "[DediManager] 데디케이티드 프로세스 띄움 - PID: " << pid << std::endl;
        }
    }

private:
    std::unordered_map<int, DediIPCSession*> _dediSessions;
};