const express = require('express');
const crypto = require('crypto');
const bcrypt = require('bcrypt');
const { redisClient } = require('../config/redisClient');
const { pool } = require('../config/mysqlClient');
const { makeResponse } = require('../utils/response');

const router = express.Router();
const saltRounds = 10;

// ==========================================================
// 회원가입 API
// ==========================================================
router.post('/signup', async (req, res) => {
    const { id, password } = req.body;
    if (!id || !password) return res.status(400).json(makeResponse(false, 400, null, { message: "ID와 Password가 필요합니다.", code: "ERR_BAD_REQUEST" }));

    try {
        const [rows] = await pool.query('SELECT login_id FROM users WHERE login_id = ?', [id]);
        if (rows.length > 0) {
            return res.status(409).json(makeResponse(false, 409, null, { message: "이미 존재하는 ID입니다.", code: "ERR_CONFLICT" }));
        }

        const hashedPassword = await bcrypt.hash(password, saltRounds);
        await pool.query('INSERT INTO users (login_id, password) VALUES (?, ?)', [id, hashedPassword]);

        // 성공 시 Redis 세션 발급 및 로그인
        const sessionId = "sess_" + crypto.randomUUID();
        await redisClient.set(sessionId, id, { EX: 3600 });
        
        res.status(201).json(makeResponse(true, 201, { sessionId }));
    } catch (error) {
        console.error("[Auth] Signup Error:", error);
        res.status(500).json(makeResponse(false, 500, null, { message: "서버 내부 오류", code: "ERR_INTERNAL" }));
    }
});

// ==========================================================
// 로그인 API
// ==========================================================
router.post('/login', async (req, res) => {
    const { id, password } = req.body;

    try {
        const [rows] = await pool.query('SELECT login_id, password FROM users WHERE login_id = ?', [id]);
        if (rows.length === 0) {
            return res.status(401).json(makeResponse(false, 401, null, { message: "존재하지 않는 ID입니다." }));
        }

        const user = rows[0];

        const isMatch = await bcrypt.compare(password, user.password);
        if (!isMatch) {
            return res.status(401).json(makeResponse(false, 401, null, { message: "비밀번호가 틀렸습니다." }));
        }

        // 성공 시 Redis 세션 발급 및 로그인
        const sessionId = "sess_" + crypto.randomUUID();
        await redisClient.set(sessionId, id, { EX: 3600 });
        
        res.status(200).json(makeResponse(true, 200, { sessionId }));
    } catch (error) {
        console.error("[Auth] Login Error:", error);
        res.status(500).json(makeResponse(false, 500, null, { message: "서버 내부 오류", code: "ERR_INTERNAL" }));
    }
});

module.exports = router;