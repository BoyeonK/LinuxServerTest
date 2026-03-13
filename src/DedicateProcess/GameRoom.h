#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "Player.h" //추후 .cpp파일로 이전, 아마 GameRoom.h와 순환참조될거임

class GameRoom {
public:
    GameRoom(int mapId, std::vector<std::string> ticketIds) : _mapId(mapId) {
        // TODO : 
        // 1. status를 "SUCCESS"로 변경 
        // 2. roomToken 발급 (유효한 플레이어인지 확인용, ticket이 있으니 별도의 token은 필요없나?)
        // 3-1. 인게임에서 사용할 인벤토리를 만듬. (Redis를 이용할지, C++코드로 해결할지 고민중)
        // 3-2. 이제 DB에서 Redis에 적혀있는 인벤토리의 수량만큼 차감 진행, IPC를 통해 HTTP서버에 전달
        auto pipe = pRedis->pipeline();

        for (const auto& ticketId : ticketIds) {
            pipe.hset(ticketId, "status", "SUCCESS");
        }
        pipe.exec();
    }

    void Clear() {

    }

private:
    int32_t _mapId;
    std::unordered_map<std::string, Player*> _players;
};