// routes/match.js
const express = require('express');
const crypto = require('crypto');
const { redisClient, scripts } = require('../config/redisClient');
const { makeResponse } = require('../utils/response');

const router = express.Router();

// 인증 미들웨어
async function requireAuth(req, res, next) {
    const sessionId = req.headers['x-session-id'];
    if (!sessionId) return res.status(401).json(makeResponse(false, 401, null, { message: "세션 ID가 없습니다." }));

    const userId = await redisClient.get(sessionId);
    if (!userId) return res.status(401).json(makeResponse(false, 401, null, { message: "만료된 세션입니다." }));

    req.userId = userId;
    next();
}

// 매치메이킹 시작
router.post('/start', requireAuth, async (req, res) => {
    const { mapId, items } = req.body;
    const ticketId = "ticket_" + crypto.randomUUID();

    await redisClient.hSet(ticketId, {
        userId: req.userId,
        status: "WAITING",
        mapId: mapId ? mapId.toString() : "0",
        items: JSON.stringify(items || [])
    });
    await redisClient.expire(ticketId, 300);

    console.log(`[매칭 큐 진입] User: ${req.userId}, Ticket: ${ticketId}`);
    res.status(200).json(makeResponse(true, 200, { ticketId }));
});

// 매치메이킹 상태 확인
router.get('/status', requireAuth, async (req, res) => {
    const ticketId = req.query.ticketId;
    if (!ticketId) return res.status(400).json(makeResponse(false, 400, null, { message: "티켓 ID가 필요합니다." }));

    const matchData = await redisClient.hGetAll(ticketId);
    if (Object.keys(matchData).length === 0) return res.status(400).json(makeResponse(false, 400, null, { message: "만료된 티켓입니다." }));

    if (matchData.status === "WAITING" && Math.random() > 0.5) {
        const ip = "127.0.0.1";
        const port = "7777";
        const token = "room_" + crypto.randomBytes(4).toString('hex');

        const result = await redisClient.eval(scripts.matchSuccess, { keys: [ticketId], arguments: [ip, port, token] });
        if (result === 1) {
            matchData.status = "SUCCESS";
            matchData.udpServerIp = ip;
            matchData.udpServerPort = port;
            matchData.roomToken = token;
        }
    }

    const responseData = { status: matchData.status };
    if (matchData.status === "SUCCESS") {
        responseData.udpServerIp = matchData.udpServerIp;
        responseData.udpServerPort = matchData.udpServerPort;
        responseData.roomToken = matchData.roomToken;
    }
    res.status(200).json(makeResponse(true, 200, responseData));
});

// 매치메이킹 취소
router.delete('/cancel', requireAuth, async (req, res) => {
    const ticketId = req.query.ticketId;
    const result = await redisClient.eval(scripts.matchCancel, { keys: [ticketId] });

    if (result === 1) {
        console.log(`[매칭 큐 이탈] Ticket: ${ticketId} 취소됨`);
        return res.status(200).json(makeResponse(true, 200, null));
    } else {
        return res.status(409).json(makeResponse(false, 409, null, { message: "이미 매칭이 성사되어 취소 불가" }));
    }
});

module.exports = router;