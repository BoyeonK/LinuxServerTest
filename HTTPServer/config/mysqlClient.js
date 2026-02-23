const mysql = require('mysql2/promise');

// 커넥션 풀
const pool = mysql.createPool({
    host: process.env.MYSQL_HOST,
    port: process.env.MYSQL_PORT,
    user: process.env.MYSQL_USER,
    password: process.env.MYSQL_PASSWORD,
    database: process.env.MYSQL_DATABASE,
    waitForConnections: true,
    connectionLimit: 5,
    queueLimit: 0
});

// 테스트하는 함수
async function testDbConnection() {
    try {
        const connection = await pool.getConnection();
        console.log('[Node.js] Connected to MySQL Database!');
        connection.release(); // 빌려온 연결을 풀에 다시 반납
    } catch (error) {
        console.error('[Node.js] MySQL Connection Error:', error.message);
    }
}

testDbConnection();

module.exports = { pool };