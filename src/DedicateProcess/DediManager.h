#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstdlib> 
#include <unordered_map>

#include "DedicateMain.h" 

// 한 프로세스에서 50명까지 감당할 예정.
struct DediServer {
    int pid;
    int fd;
    int ingamePlayers = 0;
    int GetAffordablePlayers() const { 
        return 50 - ingamePlayers; 
    }
}

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
        else if (pid == 0) {
            execl("./LinuxServerTest", "./LinuxServerTest", "--dedicated", (char*)nullptr);
            std::cerr << "exec 실패! (바이너리 경로 확인 필요)" << std::endl;
            exit(1);
        } 
        else {
            DediServer newServer;
            newServer.pid = pid;
            newServer.fd = -1; // 아직 연결되지 않았으므로 -1로 초기화 (추후 accept 시 업데이트)
            newServer.ingamePlayers = 0;

            _servers[pid] = newServer;
            std::cout << "[DediManager] 데디케이티드 프로세스 띄움 - PID: " << pid << std::endl;
        }
    }

private:
    std::unordered_map<int, DediServer> _servers;
};