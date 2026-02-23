const net = require('net');
const protobuf = require("protobufjs");
const path = require('path');

const SOCKET_PATH = '/tmp/https.sock';

const protoPath = path.join(__dirname, '../IPCProtocol.proto');

let ipcClient = null;
let HttpWelcomeType = null;

function initIPC() {
    protobuf.load(protoPath, (err, root) => {
        if (err) {
            console.warn("[Node.js IPC] Proto 로드 실패! (경로를 확인하세요)", err);
            return;
        }

        HttpWelcomeType = root.lookupType("IPC_Protocol.HttpWelcome");
        
        ipcClient = net.createConnection(SOCKET_PATH, () => {
            console.log('[Node.js IPC] Connected to C++ UDP Server via IPC!');
        });

        ipcClient.on('error', (err) => console.error('[Node.js IPC] IPC 에러:', err.message));
        
        ipcClient.on('data', (data) => {
            console.log('[Node.js IPC] IPC 데이터 수신:', data);
            // TODO : 헤더(2byte pktId, 2byte pktSize) 파싱하고 뒤의 데이터 읽어서 Protobuf로 직렬화된 명령 처리하기.
            // 추가 : 한번에 모든 패킷이 오지 않을 경우, 데이터 처리 로직 확인하기
        });
    });
}

// TODO : 매치메이킹 API 등에서 C++ 서버로 데이터를 쏠 때 사용할 전용 함수
/*
function sendToCpp(buffer) {
    if (ipcClient && !ipcClient.destroyed) {
        ipcClient.write(buffer);
    } else {
        console.error('[Node.js IPC] 통신 불가: C++ 서버와 연결되어 있지 않습니다.');
    }
}
*/

// 다른 파일에서 이 함수들을 쓸 수 있도록 내보내기
module.exports = {
    initIPC,
    //sendToCpp
};