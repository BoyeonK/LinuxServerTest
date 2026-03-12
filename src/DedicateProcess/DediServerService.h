#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../PacketHandler.h"
#include "../IPCProtocol/IPCProtocol.pb.h"
#include "DediSessions.h"
#include "GameRoom.h"

class D2MSession;

// TODO :
// 1. UDP Session 완성
// 2. 매칭된 플레이어들을 하나의 방으로 묶어 관리
// 3. 매칭된 플레이어에게 방이 할당되면 Redis에 상태변수 갱신 및 접속할 IP주소 및 포트 등록
// 4. 접속 요청된 플레이어의 정보를 저장하고, 해당 유저가 올바른 ticket을 가지고 있다면 핸들러 함수 허용

class DediServerService {
public:
    DediServerService() {};
    ~DediServerService() {
        if (_dediFd != -1) ::close(_dediFd);
    };

    bool Init() {
        if (InitMainIPC() == false)
            return false;

        if (InitUDP() == false)
            return false;

        return true;
    }

    bool InitMainIPC() {
        _dediFd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (_dediFd == -1) {
            std::cerr << "D3-1 - X : 소켓 생성 실패" << std::endl;
            return false;
        }

        struct sockaddr_un addr = {};
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, "/tmp/dedicate.sock", sizeof(addr.sun_path) - 1);

        if (connect(_dediFd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            std::cerr << "D3-1 - X : 로비 서버 Connect 실패" << std::endl;
            close(_dediFd);
            return false;
        }

        _pD2MSession = new D2MSession(_dediFd, IORing);
        std::cout << "D3-1 - OK : DediServerService 객체 초기화 완" << std::endl;

        SendIdentityPacket();
        _pD2MSession->Recv();

        return true;
    };

    void SendIdentityPacket() {
        IPC_Protocol::D2MInitComplete pkt;
        pkt.set_pid(getpid());
        std::cout << "D3-2 : IPC_Protocol::D2MInitComplete으로 직렬화한 pid 전송 시도 : " << getpid() << std::endl;
        SendBuffer* pSendBuffer = PacketHandler::MakeSendBuffer(pkt);
        _pD2MSession->Send(pSendBuffer);
    }

    bool InitUDP() {
        _udpFd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
        if (_udpFd == -1) {
            std::cerr << "D3-3 - X : UDP 소켓 생성 실패" << std::endl;
            return false;
        }

        bool bindSuccess = false;

        // TODO : 테스트용 임시 포트 범위, 나중에는 범위를 바꿀것이며 환경변수로 지정.
        constexpr int MIN_PORT = 7000;
        constexpr int MAX_PORT = 7100;

        for (int port = MIN_PORT; port <= MAX_PORT; ++port) {
            struct sockaddr_in addr = {};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            addr.sin_port = htons(port);

            if (bind(_udpFd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                _udpPort = port;
                bindSuccess = true;
                break;
            }
        }

        // 범위 내의 모든 포트가 꽉 찼을 경우
        if (!bindSuccess) {
            std::cerr << "D3-3 - X : 가용 UDP 포트가 없습니다! (최대 방 생성 개수 도달)" << std::endl;
            close(_udpFd);
            _udpFd = -1;
            return false;
        }

        std::cout << "D3-3 - OK : UDP 서버 준비 완료. EC2 통신용 포트: " << _udpPort << std::endl;

        _pClientSession = new D2CSession(_udpFd, IORing);
        _pClientSession->RegisterRecv();
        
        return true;
    }

    bool MakeRoomForThisGroup(std::vector<std::string>& ticketIds) {
        if (ticketIds.empty() || pRedis == nullptr) return false;
        auto pipe = pRedis->pipeline();
        for (const auto& ticketId : ticketIds) {
            pipe.hmget(ticketId, {"mapId", "status"});
        }
        auto replies = pipe.exec();

        std::string expectedMapId = "";
        bool isGroupValid = true;

        for (int i = 0; i < replies.size(); ++i) {
            // hmget의 결과 std::vector<OptionalString>
            auto fields = replies.get<std::vector<sw::redis::OptionalString>>(i);
            
            if (fields.size() < 2 || !fields[0] || !fields[1]) {
                std::cerr << "MakeRoomForThisGroup - X " << ticketIds[i] << std::endl;
                isGroupValid = false;
                break;
            }

            std::string mapId = *(fields[0]);
            std::string status = *(fields[1]);

            if (i == 0) {
                expectedMapId = mapId;
            } 
            else if (mapId != expectedMapId) {
                std::cerr << "MakeRoomForThisGroup 그룹 내 Map ID 불일치 (" << expectedMapId << " vs " << mapId << ")" << std::endl;
                isGroupValid = false;
                break;
            }

            if (status != "INPROGRESS") {
                std::cerr << "MakeRoomForThisGroup INPROGRESS가 아닌 상태 발견 (" << status << ")" << std::endl;
                isGroupValid = false;
                break;
            }
        }

        if (isGroupValid == false) {
            auto failPipe = pRedis->pipeline();
            for (const auto& ticketId : ticketIds) {
                failPipe.hset(ticketId, "status", "FAILED");
            }
            failPipe.exec();
            return false; 
        }

        // TODO : IP주소 환경변수에 적어놓기
        // std::string publicIp = GetPublicIP(); 
        std::string publicIp = "127.0.0.1";
        auto successPipe = pRedis->pipeline();
        
        static int32_t roomId = 0;
        roomId++;

        for (const auto& ticketId : ticketIds) {
            successPipe.hset(ticketId, "udpServerIp", publicIp);
            successPipe.hset(ticketId, "udpServerPort", std::to_string(_udpPort));
            _ticketToRoomId[ticketId] = roomId; //혹시 중복이면 덮어쓰기
        }
        successPipe.exec();

        GameRoom* newRoom = ObjectPool<GameRoom>::Acquire(std::stoi(expectedMapId), ticketIds);
        _gameRooms.insert({roomId, newRoom});

        return true;
    }

private:
    int _dediFd = -1;
    D2MSession* _pD2MSession = nullptr;
    
    int _udpFd = -1;
    D2CSession* _pClientSession = nullptr;
    uint16_t _udpPort = 0;

    //TODO : UDP로 접속할 클라이언트들의 그룹인 GameRoom을 담은 컨테이너 추가
    std::unordered_map<int32_t, GameRoom*> _gameRooms;
    std::unordered_map<std::string, int32_t> _ticketToRoomId;
};

extern DediServerService* pDediServer;