const redis = require('redis');

// Redis 클라이언트 생성
const redisClient = redis.createClient({
    url: `redis://${process.env.REDIS_HOST}:${process.env.REDIS_PORT}`
});

redisClient.on('error', (err) => console.error('HTTP서버에서 Redis에러 감지', err));

async function connectRedis() {
    try {
        await redisClient.connect();
        console.log('HTTP서버 Redis 연결 성공');
    } catch (err) {
        console.error('HTTP서버 Redis 연결 실패:', err);
    }
}

// 예제 코드!
// TODO : 
// 1. 매치메이킹 시작 State가 Lobby인 경우, State를 Lobby에서 MatchmakeProcess로 변경. + 이후 IPC를 통해 해당 세션의 매치메이킹을 진행.
// 2. 매치케이킹 캔슬 State가 MatchmakeProcess인 경우, State를 MatchmakeProcess에서 Lobby로 변경. + 이후 IPC를 통해 해당 세션의 매치메이킹 상태를 해제.
const scripts = {
    matchSuccess: `
        local currentStatus = redis.call('HGET', KEYS[1], 'status')
        if currentStatus == 'WAITING' then
            redis.call('HSET', KEYS[1], 'status', 'SUCCESS', 'udpServerIp', ARGV[1], 'udpServerPort', ARGV[2], 'roomToken', ARGV[3])
            return 1
        else
            return 0
        end
    `,
    matchCancel: `
        local currentStatus = redis.call('HGET', KEYS[1], 'status')
        if currentStatus == 'WAITING' then
            redis.call('DEL', KEYS[1])
            return 1
        elseif not currentStatus then
            return 1
        else
            return 0
        end
    `
};

module.exports = {
    redisClient,
    connectRedis,
    scripts
};