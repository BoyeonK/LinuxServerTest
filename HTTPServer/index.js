const net = require('net');
const protobuf = require("protobufjs");

const SOCKET_PATH = '/tmp/https.sock';

console.log('[Node.js] Connecting to IPC server...');

const client = net.createConnection(SOCKET_PATH, () => {
    console.log('[Node.js] Connected to C++!');
    client.write('Hello from Node.js (IPC Test)');
});

client.on('error', (err) => {
    console.error('[Node.js] Connection Error:', err.message);
    process.exit(1);
});

// 테스트 후 2초 뒤 종료
setTimeout(() => {
    client.end();
    process.exit(0);
}, 2000);

/*
setInterval(() => {}, 1000);
*/