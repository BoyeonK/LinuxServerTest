const path = require('path');
const express = require('express');
require('dotenv').config({ path: path.join(__dirname, '.env') });

const { connectRedis } = require('./config/redisClient');
const { initIPC } = require('./ipc/ipcManager');
const { makeResponse } = require('./utils/response');

const authRoutes = require('./routes/auth');
const matchRoutes = require('./routes/match');

const app = express();
const port = 3000;
app.use(express.json());

connectRedis();
initIPC();

app.get('/api/version', (req, res) => {
    res.status(200).json(makeResponse(true, 200, { latestVersion: "alphaTest", isMaintenance: false }));
});

app.use('/api', authRoutes); 
app.use('/api/game/match', matchRoutes);

// 서버 실행
app.listen(port, () => {
    console.log(`H1 - OK : HTTP서버 클라이언트의 요청 대기중`);
});