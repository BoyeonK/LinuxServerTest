#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstdlib> 
#include <unordered_map>

#include "DedicateMain.h" 
#include "../SocketWrapper.h"

class DediManager {
public:
    DediManager() = default;
    ~DediManager();

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

    void OnAcceptDedi(int DediIPCsockFd, DediTempSession* pTempSession) {
        _tempSessions[DediIPCsockFd] = pTempSession;
        pTempSession->Recv();
    }

    bool FinalizeConnection(int pid, int fd) {
        auto itTemp = _tempSessions.find(fd);
        auto itDedi = _dediSessions.find(pid);

        if (itTemp != _tempSessions.end() && itDedi != _dediSessions.end()) {
            DediTempSession* pTemp = itTemp->second;
            DediIPCSession* pReal = itDedi->second;

            pReal->BindSocket(fd);      
               
            _tempSessions.erase(itTemp);

            std::cout << "[DediManager] PID " << pid << " 연결 및 인증 완료!" << std::endl;
            return true;
        }
        
        std::cerr << "[DediManager] 인증 실패: 존재하지 않는 PID(" << pid << ") 또는 FD" << std::endl;
        return false;
    }

private:
    //key = pid, 
    std::unordered_map<int, DediIPCSession*> _dediSessions;

    //key = fd, 여기서 pid를 받은 임시 세션을 아래의 pid key의 세션과 합체
    std::unordered_map<int, DediTempSession*> _tempSessions;
};