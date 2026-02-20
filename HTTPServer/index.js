const path = require('path');
const express = require('express');
const crypto = require('crypto'); // 세션 키, 티켓 생성용 (내장 모듈)
const net = require('net');
const bcrypt = require('bcrypt');
const saltRounds = 10;
const protobuf = require("protobufjs");

require('dotenv').config({ path: path.join(__dirname, '.env') });
const protoPath = path.join(__dirname, 'IPCProtocol.proto');

const app = express();
const port = 3000;

app.use(express.json());

// ======================================================================
// 임시 데이터베이스 (추후 Redis 및 MySQL로 교체)
// ======================================================================
const dbUsers = new Map();         // { id: password }
const redisSessions = new Map();   // { sessionId: userId }
const redisMatchQueue = new Map(); // { ticketId: { userId, status, ... } }

// ======================================================================
// 공통 응답 생성
// ======================================================================
function makeResponse(success, code, data = null, error = null) {
    return { success, code, data, error };
}

// ======================================================================
// 인증 미들웨어
// ======================================================================
function requireAuth(req, res, next) {
    const sessionId = req.headers['x-session-id'];

    if (!sessionId || !redisSessions.has(sessionId)) {
        return res.status(401).json(makeResponse(false, 401, null, {
            message: "유효하지 않거나 만료된 세션입니다.",
            code: "ERR_UNAUTHORIZED"
        }));
    }

    // 통과! (다음 라우터에서 쓸 수 있게 req에 유저 ID 세팅)
    req.userId = redisSessions.get(sessionId);
    next();
}

// ======================================================================
// REST API
// ======================================================================

// 최초 접속 시, 버전 및 점검중 여부 확인 API
app.get('/api/version', (req, res) => {
    res.status(200).json(makeResponse(true, 200, {
        latestVersion: "1.0.5",
        isMaintenance: false
    }));
});

// 계정 생성 API
app.post('/api/signup', async (req, res) => {
    const { id, password } = req.body;

    if (!id || !password) {
        return res.status(400).json(makeResponse(false, 400, null, { message: "ID와 Password가 필요합니다.", code: "ERR_BAD_REQUEST" }));
    }

    if (dbUsers.has(id)) {
        return res.status(409).json(makeResponse(false, 409, null, { message: "이미 존재하는 ID입니다.", code: "ERR_CONFLICT" }));
    }

    const hashedPassword = await bcrypt.hash(password, saltRounds);

    // 자동 로그인 처리 (세션 발급)
    const sessionId = "sess_" + crypto.randomUUID();
    redisSessions.set(sessionId, id);

    res.status(201).json(makeResponse(true, 201, { sessionId }));
});

// 로그인 API
app.post('/api/login', async (req, res) => {
    const { id, password } = req.body;

    // TODO : salt치고 입력된 비밀번호와 해싱한 이후, 이 결과값과 DB의 해시바이너리를 비교해야 함.
    const savedHashedPassword = dbUsers.get(id);
    if (!savedHashedPassword) {
        return res.status(401).json(makeResponse(false, 401, null, { message: "존재하지 않는 ID입니다." }));
    }

    // bcrypt.compare가 입력받은 평문과 DB의 해시+솔트 값을 안전하게 비교해 줍니다.
    const isMatch = await bcrypt.compare(password, savedHashedPassword);

    if (!isMatch) {
        return res.status(401).json(makeResponse(false, 401, null, { message: "비밀번호가 틀렸습니다." }));
    }

    const sessionId = "sess_" + crypto.randomUUID();
    redisSessions.set(sessionId, id);

    res.status(200).json(makeResponse(true, 200, { sessionId }));
});

// 매치메이킹 시작 (티켓 발급)
app.post('/api/game/match/start', requireAuth, (req, res) => {
    const { mapId, items } = req.body;
    const userId = req.userId;

    // 티켓 생성 및 대기열(Redis)에 저장
    const ticketId = "ticket_" + crypto.randomUUID();
    redisMatchQueue.set(ticketId, {
        userId: userId,
        status: "WAITING", // 초기 상태
        mapId: mapId,
        items: items
    });

    console.log(`[매칭 큐 진입] User: ${userId}, Ticket: ${ticketId}`);

    res.status(200).json(makeResponse(true, 200, { ticketId }));
});

// 매치메이킹 상태 확인 (Polling)
app.get('/api/game/match/status', requireAuth, (req, res) => {
    const ticketId = req.query.ticketId;

    if (!ticketId || !redisMatchQueue.has(ticketId)) {
        return res.status(400).json(makeResponse(false, 400, null, { message: "유효하지 않은 티켓입니다.", code: "ERR_INVALID_TICKET" }));
    }

    const matchData = redisMatchQueue.get(ticketId);

    // 테스트용 임시 로직
    if (matchData.status === "WAITING" && Math.random() > 0.5) {
        matchData.status = "SUCCESS";
        matchData.udpServerIp = "127.0.0.1";
        matchData.udpServerPort = 7777;
        matchData.roomToken = "room_" + crypto.randomBytes(4).toString('hex');
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
app.delete('/api/game/match/cancel', requireAuth, (req, res) => {
    const ticketId = req.query.ticketId;
    const matchData = redisMatchQueue.get(ticketId);

    if (!matchData) {
        return res.status(200).json(makeResponse(true, 200, null)); // 없으면 성공으로 간주
    }

    if (matchData.status === "SUCCESS") {
        return res.status(409).json(makeResponse(false, 409, null, { message: "이미 매칭이 성사되어 취소 불가", code: "ERR_ALREADY_MATCHED" }));
    }

    // 큐에서 삭제 (취소)
    redisMatchQueue.delete(ticketId);
    console.log(`[매칭 큐 이탈] Ticket: ${ticketId} 취소됨`);

    res.status(200).json(makeResponse(true, 200, null));
});

// ======================================================================
// C++ 서버와의 IPC (백그라운드 유지)
// ======================================================================
const SOCKET_PATH = '/tmp/https.sock';
protobuf.load(protoPath, (err, root) => {
    if (err) {
        console.warn("[Node.js] Proto 로드 실패! (API 서버는 계속 실행됩니다)");
    } else {
        const HttpWelcome = root.lookupType("IPC_Protocol.HttpWelcome");
        const client = net.createConnection(SOCKET_PATH, () => {
            console.log('[Node.js] Connected to C++ UDP Server via IPC!');
        });

        client.on('error', (err) => console.error('[Node.js] IPC 에러:', err.message));
        client.on('data', (data) => console.log('[Node.js] IPC 데이터 수신:', data));
    }
});

// ======================================================================
// 서버 실행
// ======================================================================
app.listen(port, () => {
    console.log(`=================================`);
    console.log(`Game API Server Running`);
    console.log(`Port: http://localhost:${port}`);
    console.log(`=================================`);
});