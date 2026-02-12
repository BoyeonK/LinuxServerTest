#pragma once

#include <iostream>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <liburing.h>

using namespace std;

#define PORT 8080
#define QUEUE_DEPTH 256
#define MESSAGE "Hello from io_uring Server!\n"

enum OpType {
    OP_ACCEPT,
    OP_SEND
};

// 각 요청마다 문맥(Context)을 저장할 구조체
struct Request {
    OpType type;
    int client_fd;
    // 필요한 경우 버퍼 포인터 등을 여기에 추가
};

// 전역 변수로 링과 리슨 소켓 관리 (예제 편의상)
struct io_uring ring;
int server_fd;
struct sockaddr_in client_addr;
socklen_t client_len = sizeof(client_addr);

// [함수] Accept 요청을 준비해서 커널에 제출
void add_accept_request() {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    
    // Accept 요청 컨텍스트 생성 (메모리 해제 주의: 여기서는 예제라 간단히 처리)
    Request* req = new Request;
    req->type = OP_ACCEPT;

    // fd: 서버 소켓, addr: 클라이언트 주소 저장될 곳, len: 길이
    io_uring_prep_accept(sqe, server_fd, (struct sockaddr*)&client_addr, &client_len, 0);
    io_uring_sqe_set_data(sqe, req); // 태그 달기
    
    io_uring_submit(&ring);
}

// [함수] Send 요청을 준비해서 커널에 제출
void add_send_request(int client_fd) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    
    Request* req = new Request;
    req->type = OP_SEND;
    req->client_fd = client_fd;

    // fd: 클라이언트 소켓, buf: 보낼 데이터, len: 길이
    io_uring_prep_send(sqe, client_fd, MESSAGE, strlen(MESSAGE), 0);
    io_uring_sqe_set_data(sqe, req); // 태그 달기

    io_uring_submit(&ring);
}

int asdf() {
    // 1. io_uring 초기화
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        perror("io_uring_queue_init");
        return 1;
    }

    // 2. 서버 소켓 설정 (Socket -> Bind -> Listen)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // [중요] 서버 재시작 시 "Address already in use" 방지
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return 1;
    }

    cout << "[Server] Listening on port " << PORT << "..." << endl;

    // 3. 최초의 Accept 요청 제출 (낚싯대 드리우기)
    add_accept_request();

    // 4. 이벤트 루프 (무한 반복)
    while (true) {
        struct io_uring_cqe *cqe;
        
        // 완료된 작업이 있는지 대기 (Blocking)
        if (io_uring_wait_cqe(&ring, &cqe) < 0) {
            perror("io_uring_wait_cqe");
            break;
        }

        // 태그(user_data)를 꺼내서 무슨 작업인지 확인
        Request* req = (Request*)io_uring_cqe_get_data(cqe);

        if (req->type == OP_ACCEPT) {
            // [Accept 완료]
            int client_fd = cqe->res; // res에는 새로운 소켓 fd가 들어있음
            
            if (client_fd >= 0) {
                cout << "[Client] Connected! (fd: " << client_fd << ")" << endl;
                
                // 1) 연결된 클라이언트에게 메시지 전송 요청
                add_send_request(client_fd);
            }

            // 2) [중요] 다음 클라이언트를 받기 위해 Accept 다시 등록 (리필)
            add_accept_request();
            
        } else if (req->type == OP_SEND) {
            // [Send 완료]
            int bytes_sent = cqe->res;
            cout << "[Send] Sent " << bytes_sent << " bytes to fd " << req->client_fd << endl;

            // 할 일 다 했으니 연결 종료
            close(req->client_fd);
        }

        // CQE 처리 완료 표시 및 Request 메모리 해제
        io_uring_cqe_seen(&ring, cqe);
        delete req;
    }

    close(server_fd);
    io_uring_queue_exit(&ring);
    return 0;
}