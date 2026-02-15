#include "IPCSocket.h"

#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <cstring>

void RunIpcTest() {
    const char* sockPath = "/tmp/game.sock";
    int serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
    
    //기존 소켓 파일 제거
    unlink(sockPath);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sockPath, sizeof(addr.sun_path) - 1);

    if (bind(serverSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Bind failed");
        return;
    }
    listen(serverSock, 1);
    std::cout << "[C++] IPC Server Waiting for Node.js..." << std::endl;

    // 테스트를 위해 블로킹
    int clientSock = accept(serverSock, NULL, NULL);
    if (clientSock == -1) {
        perror("Accept failed");
        return;
    }
    std::cout << "[C++] Node.js Connected!" << std::endl;

    char buffer[1024];
    ssize_t len = read(clientSock, buffer, sizeof(buffer) - 1);
    if (len > 0) {
        buffer[len] = '\0';
        std::cout << "[C++] Received from Node.js: " << buffer << std::endl;
    }

    close(clientSock);
    close(serverSock);
    unlink(sockPath);
}