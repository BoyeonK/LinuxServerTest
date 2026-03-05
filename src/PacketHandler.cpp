#include "PacketHandler.h"
#include <iostream>
#include <string>

using namespace std;

std::function<bool(Session*, unsigned char*, int32_t)> GProtoPacketHandler[UINT16_MAX];

bool Handle_Invalid(Session* pSession, unsigned char* buffer, int32_t len) {
#ifdef _DEBUG
    cout << "Something goes wrong, Client sent invalid packet" << endl;
#endif
    return false;
}

bool Handle_H2M_Welcome(Session* pSession, IPC_Protocol::H2MWelcome& pkt) {
    cout << "H3-2 - OK 받은 echo_message : " << pkt.echo_message() << endl;

    return true;
}

bool Handle_H2M_MatchMake(Session* pSession, IPC_Protocol::H2MMatchMake& pkt) {
    // TODO : 
    // 1. pkt에서 ticket_redis_key를 읽는다.
    // 2. 해당 key로 Redis에 접근하여 table을 읽고, MatchTicket struct를 pool에서 할당받고 읽은 값을 토대로 초기화.
    // 3. Matchmaker의 AddSingleMatchTicket으로 대기열에 추가한다.
    string ticketKey = pkt.ticket_redis_key();
    try {
        auto val = pRedis->hgetall(ticketKey);
        if (val.empty()) return false;

        MatchTicket* pTicket = ObjectPool<MatchTicket>::Acquire(
            ticketKey,
            std::stoi(val.at("uId")),
            std::stoi(val.at("aggression")),
            std::stoi(val.at("mapId"))
        );
        
        if (pDediManager->AddSingleMatchTicket(pTicket) == false) {
            std::cerr << "[MatchHandler] 매치 대기열 추가 실패" << std::endl;
            ObjectPool<MatchTicket>::Release(pTicket);
            return false;
        }

    } catch (const sw::redis::Error& e) {
        std::cerr << "[MatchHandler] Redis Error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool Handle_H2M_MatchMakeCancel(Session* pSession, IPC_Protocol::H2MMatchMakeCancel& pkt) {
    cout << "매치취소 요청 테스트용 콘솔 출력" << endl;

    return true;
}

bool Handle_D2M_InitComplete(Session* pSession, IPC_Protocol::D2MInitComplete& pkt) {
    cout << "매치취소 요청 테스트용 콘솔 출력" << endl;

    return true;
}