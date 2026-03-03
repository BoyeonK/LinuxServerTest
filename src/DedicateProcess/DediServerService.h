#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

class MainIPCSession;

class DediServerService {
public:
    DediServerService() {};
    ~DediServerService() {
        if (_dediFd != -1) ::close(_dediFd);
    };

    bool Init() {
        if (InitMainIPC() == false)
            return false;
        /*
        if (InitUDP() == false)
            return false;
        */ 
        return true;
    }

    bool InitMainIPC() {
        _dediFd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (_dediFd == -1) {
            std::cerr << "[Dedicated] 소켓 생성 실패" << std::endl;
            return false;
        }

        struct sockaddr_un addr = {};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, "/tmp/dedicate.sock", sizeof(addr.sun_path) - 1);

        if (connect(_dediFd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            std::cerr << "[Dedicated] 로비 서버 연결 실패 (Connect error)" << std::endl;
            close(_dediFd);
            return false;
        }

        _mainSession = new MainIPCSession(_dediFd, IORing);
        _mainSession->Recv();

        // TODO : 
        // mainSession에 실행된 pid전송하기

        return true;
    };

    /*
    bool InitUDP() {

    };
    */

private:
    int _dediFd = -1;
    MainIPCSession* _mainSession = nullptr;

    //TODO : UDP로 접속한 클라이언트들의 Session을 담은 컨테이너 추가
};

extern DediServerService* pDediServer;