#include "Matchmaker.h"

#include <algorithm>
#include <iterator>
#include <utility>
#include <iostream>
#include <sw/redis++/redis++.h>
#include "../GlobalVariable.h"
#include "../ObjectPool.h"

// ipc를 통해 받은 ticket에 해당하는 구조체를 대기열에 추가할 함수
// 현재 매치메이킹 프로세스(메인 프로세스)는 싱글스레드로 동작할 예정이므로 mutex를 포함한 동기화 사용 X
void MatchMaker::AddSingleMatchTicket(MatchTicket* pTicket) {
    _ticketsToAdd.emplace_back(pTicket);
}

void MatchMaker::AddNewMatchTickets() {
    // 1. _ticketsToAdd를 먼저 들어온 친구가 앞에 오도록 정렬
    // 2. 정렬된 _ticketsToAdd를 이 시점에 _timeSortedTicketVector와 병합
    // 3. 앞에서부터 순회하면서 agression을 읽고, _bucket[agression]에 뒤에서부터 꽂음
        // (_bucket의 맨 뒤가 _ticketsToAdd의 맨 앞보다 먼저 온 친구라는것을 보장 해 줄 예정)

    if (_ticketsToAdd.empty()) return;

    std::stable_sort(_ticketsToAdd.begin(), _ticketsToAdd.end(), 
        [](const MatchTicket* a, const MatchTicket* b) {
            return a->startTime < b->startTime;
        }
    );

    for (MatchTicket* ticket : _ticketsToAdd) {
        _bucket[ticket->aggression].push_back(ticket);
    }

    // 1-2. 정렬된 _ticketsToAdd를 _timeSortedTicketVector와 병합 (O(N) 속도!)
    TicketVector newMainQueue;
    newMainQueue.reserve(_timeSortedTicketVector.size() + _ticketsToAdd.size()); // 메모리 재할당 방지

    std::merge(
        _timeSortedTicketVector.begin(), _timeSortedTicketVector.end(),
        _ticketsToAdd.begin(), _ticketsToAdd.end(),
        std::back_inserter(newMainQueue),
        [](const MatchTicket* a, const MatchTicket* b) {
            return a->startTime < b->startTime;
        }
    );

    _timeSortedTicketVector = std::move(newMainQueue);
    _ticketsToAdd.clear();
}

void MatchMaker::AddRematchTickets() {
    // 1. ticketsToRematch의 모든 TicketVector를 먼저 온 친구가 앞에 오도록 정렬함
    // 2. 모든 agression마다, 정렬된 _bucket[agression]과, 정렬된 _ticket[agression]를 병합
    // 3. ticketsToRematch의 모든 TicketVector의 ticket을 모아 정렬하여(하나하나 정렬하는거 보다 이게 나음) _timeSortedTicketVector와 병합.
    TicketVector allRematchTickets;

    for (int aggression = 0; aggression <= _maxAgression; ++aggression) {
        TicketVector& rematchVec = _ticketsToRematch[aggression];

        if (rematchVec.empty()) continue;

        std::stable_sort(rematchVec.begin(), rematchVec.end(),
            [](const MatchTicket* a, const MatchTicket* b) {
                return a->startTime < b->startTime;
            }
        );

        TicketVector& bucketVec = _bucket[aggression];
        TicketVector mergedBucket;
        mergedBucket.reserve(bucketVec.size() + rematchVec.size());

        std::merge(
            bucketVec.begin(), bucketVec.end(),
            rematchVec.begin(), rematchVec.end(),
            std::back_inserter(mergedBucket),
            [](const MatchTicket* a, const MatchTicket* b) {
                return a->startTime < b->startTime;
            }
        );

        bucketVec = std::move(mergedBucket);
        allRematchTickets.insert(allRematchTickets.end(), rematchVec.begin(), rematchVec.end());

        rematchVec.clear();
    }

    if (!allRematchTickets.empty()) {
        std::stable_sort(allRematchTickets.begin(), allRematchTickets.end(),
            [](const MatchTicket* a, const MatchTicket* b) {
                return a->startTime < b->startTime;
            });

        TicketVector newMainQueue;
        newMainQueue.reserve(_timeSortedTicketVector.size() + allRematchTickets.size());

        std::merge(
            _timeSortedTicketVector.begin(), _timeSortedTicketVector.end(),
            allRematchTickets.begin(), allRematchTickets.end(),
            std::back_inserter(newMainQueue),
            [](const MatchTicket* a, const MatchTicket* b) {
                return a->startTime < b->startTime;
            }
        );

        _timeSortedTicketVector = std::move(newMainQueue);
    }
}

void MatchMaker::DeleteUsedOrUnvaildTickets() {
    // 1. _bucket과 _timeSortedTicketVector에 들어있는, isMatched가 true인 포인터를 '놓아준다'.
    // 2. _ticketsToDelete에 해당하는 Redis의 ticket을 파기함. (없을 수도 있음. HTTP서버 선에서 매칭 취소로 인해 잘라냈을 수 있음)
    // 3. _ticket의 내용을 비우고 Pool에 반환
    _timeSortedTicketVector.erase(
        std::remove_if(_timeSortedTicketVector.begin(), _timeSortedTicketVector.end(),
            [](const MatchTicket* t) { return t->isMatched; }),
        _timeSortedTicketVector.end()
    );

    for (TicketVector& bucketVec : _bucket) {
        if (bucketVec.empty()) continue;
        
        bucketVec.erase(
            std::remove_if(bucketVec.begin(), bucketVec.end(),
                [](const MatchTicket* t) { return t->isMatched; }),
            bucketVec.end()
        );
    }

    if (_ticketsToDelete.empty()) return;
    std::vector<std::string> keysToDelete;
    keysToDelete.reserve(_ticketsToDelete.size());

    for (MatchTicket* pTicket : _ticketsToDelete) {
        if (pTicket != nullptr) {
            keysToDelete.push_back(std::move(pTicket->ticketId));

            ObjectPool<MatchTicket>::Release(pTicket);
        }
    }

    if (!keysToDelete.empty() && pRedis != nullptr) {
        try {
            pRedis->del(keysToDelete.begin(), keysToDelete.end());
        } catch (const sw::redis::Error& e) {
            std::cerr << "대기열 처리 중 에러 발생 - Ticket소거 과정: " << e.what() << std::endl;
        }
    }

    _ticketsToDelete.clear();
}

void MatchMaker::FindMatchGroup() {
    _matchedGroups.clear();
    std::fill(_pivotMemorizationFlags.begin(), _pivotMemorizationFlags.end(), false);

    for (MatchTicket* pivot : _timeSortedTicketVector) {
        if (pivot->isMatched) continue;

        int pivotAggr = pivot->aggression;

        if (_pivotMemorizationFlags[pivotAggr]) continue;

        int waitTime = pivot->GetWaitTimeSeconds();
        int allowedDiff = 0; 
        int targetMinPlayers = 4;

        if (waitTime >= 60) {
            allowedDiff = 1;
            targetMinPlayers = 1;
        } else if (waitTime >= 40) {
            allowedDiff = 1;
            targetMinPlayers = 2;
        } else if (waitTime >= 20) {
            allowedDiff = 0;
            targetMinPlayers = 3;
        }

        TicketVector matchedGroup;
        matchedGroup.reserve(4);
        matchedGroup.push_back(pivot);

        int searchMin = std::max(0, pivotAggr - allowedDiff);
        int searchMax = std::min(_maxAgression, pivotAggr + allowedDiff);

        int searchRange = 0;
        while (searchRange <= allowedDiff) {
            if (searchRange == 0) {
                for (MatchTicket* candidate : _bucket[pivotAggr]) {
                    if (matchedGroup.size() == 4) break;
                    if (candidate == pivot || candidate->isMatched) continue;
                    matchedGroup.push_back(candidate);
                }
            } else {
                int uaggr = pivotAggr + searchRange;
                int daggr = pivotAggr - searchRange;

                if (uaggr <= searchMax) {
                    for (MatchTicket* candidate : _bucket[uaggr]) {
                        if (matchedGroup.size() == 4) break;
                        if (candidate->isMatched) continue;
                        matchedGroup.push_back(candidate);
                    }
                }              

                if (matchedGroup.size() < 4 && daggr >= searchMin) {
                    for (MatchTicket* candidate : _bucket[daggr]) { 
                        if (matchedGroup.size() == 4) break;
                        if (candidate->isMatched) continue;
                        matchedGroup.push_back(candidate);
                    }
                }
            }   
            
            if (matchedGroup.size() == 4) break;
            searchRange++;
        }

        if (matchedGroup.size() >= targetMinPlayers) {
            for (MatchTicket* member : matchedGroup)
                member->isMatched = true;

            _matchedGroups.push_back(std::move(matchedGroup));          
        } else {
            _pivotMemorizationFlags[pivotAggr] = true;
        }
    }
}

bool MatchMaker::VerifyAndSetMatchStatus(const TicketVector& matchedGroup) {
    if (matchedGroup.empty() || pRedis == nullptr) return false;

    std::string luaScript = R"(
        for i = 1, #KEYS do
            local status = redis.call('HGET', KEYS[i], 'status')
            if status ~= 'WAITING' then
                return 0
            end
        end
        
        for i = 1, #KEYS do
            redis.call('HSET', KEYS[i], 'status', ARGV[1])
        end
        return 1
    )";

    std::vector<std::string> keys;
    keys.reserve(matchedGroup.size());
    for (MatchTicket* ticket : matchedGroup) {
        // TODO : 형식 확인 필요
        keys.push_back("ticket_" + ticket->ticketId); 
    }

    std::vector<std::string> args = {"INPROGRESS"}; 

    try {
        long long result = pRedis->eval<long long>(
            luaScript, 
            keys.begin(), keys.end(), 
            args.begin(), args.end()
        );
        
        return result == 1; // 1이면 전원 성공, 0이면 누군가 취소함
        
    } catch (const sw::redis::Error& e) {
        std::cerr << "[Redis Error] Lua 스크립트 실행 실패: " << e.what() << std::endl;
        return false;
    }
}

void MatchMaker::StartMatchMakeInternal() {
    for (auto& ticketVec : _matchedGroups) {
        bool isMatchValid = VerifyAndSetMatchStatus(ticketVec);
        if (isMatchValid) {
            pDediManager->DistributePlayerGroup(ticketVec);
        } else {
            for (MatchTicket* ticket : ticketVec) {
                auto statusOpt = pRedis->hget("ticket_" + ticket->ticketId, "status");

                if (statusOpt && *statusOpt == "WAITING") {
                    ticket->isMatched = false;
                    _ticketsToRematch[ticket->aggression].push_back(ticket);
                } else {
                    _ticketsToDelete.push_back(ticket);
                }
            }
        }       
    }
}

void MatchMaker::StartMatchMake() {
    AddRematchTickets();
    AddNewMatchTickets();
    FindMatchGroup();
    StartMatchMakeInternal();
    DeleteUsedOrUnvaildTickets();
}
