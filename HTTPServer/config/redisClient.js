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

module.exports = {
    redisClient,
    connectRedis,
};