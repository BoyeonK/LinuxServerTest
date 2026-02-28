#pragma once

struct MatchTicket {
    std::string ticketId;
    std::string uid;
    int aggression;
    int mapId;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    
    // 지연 삭제(Lazy Deletion)용 플래그
    bool isMatched = false; 

    int GetWaitTimeSeconds() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
    }
};

using std::list<MatchTicket*> TicketList;
using std::vector<MatchTicket*> TicketVector;

//map별로 MatchMaker를 만들 예정
class MatchMaker {
public:
    MatchMaker(int32_t mapId) : _mapId(mapId) {}

    // ipc를 통해 받은 ticket에 해당하는 구조체를 대기열에 추가할 함수
    // 현재 매치메이킹 프로세스(메인 프로세스)는 싱글스레드로 동작할 예정이므로 mutex를 포함한 동기화 사용 X
    void AddSingleMatchTicket(MatchTicket* pTicket) {
        _ticketsToAdd.emplace_back(pTicket);
    }

    void AddNewMatchTickets() {
        // TODO : 
        // 1. _ticketsToAdd를 먼저 들어온 친구가 앞에 오도록 정렬
            // 1-2. 제미니 형님의 제안 : 정렬된 _ticketsToAdd를 이 시점에 _timeSortedTicketVector와 병합
            // StartMatchMake() 시점에서 1번 과정 생략 가능하며, 이쪽이 성능상 우위
        // 2. 앞에서부터 순회하면서 agression을 읽고, _bucket[agression]에 뒤에서부터 꽂음
        // (_bucket의 맨 뒤가 _ticketsToAdd의 맨 앞보다 먼저 온 친구라는것을 보장 해 줄 예정)
    }

    void AddRematchTickets() {
        // TODO : 
        // 1. ticketsToRematch의 모든 TicketVector를 먼저 온 친구가 앞에 오도록 정렬함
        // 2. 모든 agression마다, 정렬된 _bucket[agression]과, 정렬된 _ticket[agression]를 병합
    }

    void DeleteUsedOrUnvaildTickets() {
        // TODO :
        // 1. _ticketsToDelete에 해당하는 Redis의 ticket을 파기함. (없을 수도 있음. HTTP서버 선에서 매칭 취소로 인해 잘라냈을 수 있음)
        // 2. _ticket의 내용을 비우고 Pool에 반환
    }

    //주기적으로 실행할 진짜 매치 함수
    void StartMatchMake() {
        DeleteUsedOrUnvaildTickets();
        AddRematchTickets();
        AddNewMatchTickets();

        // TODO :
        // 1. _bucket 안의 모든 요소를 순서대로 _timeSortedTicketVector에 와바박 넣고 먼저 큐를 시도한 친구가 앞에 오도록 정렬한 뒤에 차례로 매치 시도
        // 2. agression x인 요소의 매칭이 실패한 경우, x를 기억했다가 다음 x에 대해서는 진행하지 않음.
        // 3. 각 _bucket에서의 순서가, _timeSortedTicketVector에 들어갔을 경우, 꼬이지 않았다는 것을 전제로 함.
    }

private:
    int32_t _mapId;
    TicketVector _ticketsToAdd;
    TicketVector _ticketsToDelete;
    
    TicketVector _timeSortedTicketVector;

    // key=agression 별 분류
    std::unordered_map<int, TicketVector> _bucket;
    std::unordered_map<int, TicketVector> _ticketsToRematch;
};