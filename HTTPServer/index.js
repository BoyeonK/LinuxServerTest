const net = require('net');
const protobuf = require("protobufjs");

const SOCKET_PATH = '/tmp/https.sock';

// [수정] 파일 이름만 적으면 실행 위치(build 폴더)에서 찾습니다.
protobuf.load("IPCProtocol.proto", (err, root) => {
    if (err) {
        console.error("[Node.js] Proto 로드 실패! 실행 위치(CWD) 확인:", process.cwd());
        throw err;
    }

    const HttpWelcome = root.lookupType("IPC_Protocol.HttpWelcome");

    const client = net.createConnection(SOCKET_PATH, () => {
        console.log('[Node.js] Connected to C++!');
        
        let count = 0;
        const maxCount = 6; // 딱 6번만 전송
    
        const timer = setInterval(() => {
            count++;
            
            // 데이터 생성
            const payload = { echoMessage: count };
            const message = HttpWelcome.create(payload);
            const buffer = HttpWelcome.encode(message).finish();
            
            const totalSize = 4 + buffer.length;
            const packet = Buffer.alloc(totalSize);
            
            packet.writeUInt16LE(1, 0);       // ID: 1
            packet.writeUInt16LE(totalSize, 2); // Size
            buffer.copy(packet, 4);           // Payload
            
            // 전송
            client.write(packet);
            console.log(`[Node.js] 패킷 전송 완료 (${count}/${maxCount})`);

            // 6번 전송 완료 시 타이머 종료 및 연결 해제
            if (count >= maxCount) {
                console.log('[Node.js] 모든 전송 완료. 1초 뒤 연결을 종료합니다.');
                clearInterval(timer);
                
                // 전송된 패킷이 C++에 도착할 시간을 잠시 주기 위해 1초 뒤 종료
                setTimeout(() => {
                    client.end(); // 소켓 연결 종료 (FIN 패킷 전송)
                }, 1000);
            }
        }, 1000);
    });

    client.on('end', () => {
        console.log('[Node.js] C++ 서버로부터 연결 종료됨');
        process.exit(0);
    });

    client.on('error', (err) => {
        console.error('[Node.js] 에러 발생:', err.message);
        process.exit(1);
    });

    client.on('data', (data) => {
        // C++에서 응답을 보낼 경우 여기서 확인 가능합니다.
        console.log('[Node.js] Received from C++:', data);
    });
});

setInterval(() => {}, 1000);
