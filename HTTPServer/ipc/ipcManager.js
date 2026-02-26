const net = require('net');
const protobuf = require("protobufjs");
const path = require('path');

const SOCKET_PATH = '/tmp/https.sock';
const protoPath = path.join(__dirname, '../IPCProtocol.proto');

const PKT_ID_MAIN_WELCOME = 1;
const PKT_ID_HTTP_WELCOME = 2;
const PKT_ID_HTTP_MATCHMAKE = 3;

let ipcClient = null;
let rootProto = null; // Protobuf Root 객체 저장용

let receiveBuffer = Buffer.alloc(0);

function initIPC() {
    protobuf.load(protoPath, (err, root) => {
        if (err) {
            console.error("IPCProtocol.proto 경로 잘못됨.", err);
            return;
        }
        
        rootProto = root;
        ipcClient = net.createConnection(SOCKET_PATH, () => {
            console.log('HTTP -> 메인프로세스 IPC 연결 성공');
            
            // 연결 테스트용 Welcome 패킷 전송
            sendHttpWelcome(1234);
        });

        ipcClient.on('error', (err) => console.error('IPC 에러:', err.message));
    
        // ==========================================================
        // 수신부: TCP 패킷 조립 및 파싱
        // ==========================================================
        ipcClient.on('data', (data) => {
            receiveBuffer = Buffer.concat([receiveBuffer, data]);

            while (receiveBuffer.length >= 4) {    
                const pktId = receiveBuffer.readUInt16LE(0); 
                const totalPacketSize = receiveBuffer.readUInt16LE(2); 

                // 헤더 포함 바이트인데 4보다 작다고? 이거 무조건 버그임
                if (totalPacketSize < 4) {
                    console.error(`IPC에러, 패킷 크기가 4바이트 미만이라고 '주장'하는 패킷이 들어옴: ${totalPacketSize}`);
                    receiveBuffer = Buffer.alloc(0); // 버퍼를 강제 초기화하여 오염된 스트림 폐기
                    break;
                }

                // 전체 패킷 크기만큼 아직 데이터가 다 도착하지 않았다면 대기
                if (receiveBuffer.length < totalPacketSize) {
                    break; 
                }

                // 패킷 크기만큼 들어왔다면 그만큼 잘라냄.
                const payload = receiveBuffer.subarray(4, totalPacketSize);

                // 추출한 페이로드 처리
                handleIncomingPacket(pktId, payload);

                // 처리가 끝난 패킷 버림
                receiveBuffer = receiveBuffer.subarray(totalPacketSize);
            }
        });
    });
}

// C++로부터 수신된 패킷 처리 라우터. 그런데 설계상 C++에서 HTTP서버로 뭘 보낼 일이 거의 없긴함..
function handleIncomingPacket(pktId, payload) {
    try {
        if (pktId === PKT_ID_MAIN_WELCOME) {
            const MainWelcome = rootProto.lookupType("IPC_Protocol.MainWelcome");
            const message = MainWelcome.decode(payload);
            console.log(`[Node.js IPC] 수신: MainWelcome (echo: ${message.echo_message})`);
        } else {
            console.warn(`[Node.js IPC] 알 수 없는 패킷 ID 수신: ${pktId}`);
        }
    } catch (err) {
        console.error(`[Node.js IPC] 패킷 디코딩 실패 (ID: ${pktId}):`, err);
    }
}

// ==========================================================
// 송신부: 헤더 조립 및 바이너리 전송
// ==========================================================
function sendToCpp(buffer) {
    if (ipcClient && !ipcClient.destroyed) {
        ipcClient.write(buffer);
    } else {
        console.error('IPC 통신 불가: C++ 서버와 연결되어 있지 않습니다.');
    }
}

// 공통 패킷 생성 헬퍼 (Header 4byte + Payload)
function makePacket(pktId, payloadBuffer) {
    const header = Buffer.alloc(4);
    const totalSize = 4 + payloadBuffer.length; 

    header.writeUInt16LE(pktId, 0);
    header.writeUInt16LE(totalSize, 2);

    return Buffer.concat([header, payloadBuffer]);
}

function sendHttpMatchMake(ticketId) {
    if (!rootProto) {
        console.error("[Node.js IPC] Proto 로드 전입니다.");
        return;
    }

    // 직렬화하고 헤더 부착
    const HttpMatchMake = rootProto.lookupType("IPC_Protocol.HttpMatchMake");
    const message = HttpMatchMake.create({ ticket_redis_key: ticketId });
    const payload = HttpMatchMake.encode(message).finish();
    const packet = makePacket(PKT_ID_HTTP_MATCHMAKE, payload);

    sendToCpp(packet);

    // TODO : 이거 빌드할때는 주석처리하든가 지워야됨
    console.log(`IPC: HttpMatchMake (ticket: ${ticketId})`);
}

function sendHttpWelcome(echoNum) {
    if (!rootProto) return;
    const HttpWelcome = rootProto.lookupType("IPC_Protocol.HttpWelcome");
    const payload = HttpWelcome.encode(HttpWelcome.create({ echo_message: echoNum })).finish();
    sendToCpp(makePacket(PKT_ID_HTTP_WELCOME, payload));
}

// 다른 파일에서 이 함수들을 쓸 수 있도록 내보내기
module.exports = {
    initIPC,
    sendHttpMatchMake
};