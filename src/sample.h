// 1. 커널이 데이터를 채워줄 "바구니" (Session이 들고 있는 버퍼)
char session_buffer[1024]; 

// 2. 읽기 요청 등록 (SQE 작성)
void request_read(int fd, struct io_uring *ring) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

    // [중요] 커널에게 "이 fd에서 데이터를 읽어서, 이 주소(session_buffer)에 담아줘"라고 예약합니다.
    io_uring_prep_read(sqe, fd, session_buffer, sizeof(session_buffer), 0);

    // [중요] 나중에 이 작업이 끝났을 때 누군지 알아보기 위해 '이름표(Task)'를 붙입니다.
    io_uring_sqe_set_data(sqe, my_read_task_ptr);

    io_uring_submit(ring);
}

// 3. 완료 확인 (CQE 확인)
void check_completion(struct io_uring *ring) {
    struct io_uring_cqe *cqe;
    
    // 완료된 작업이 있는지 봅니다.
    if (io_uring_peek_cqe(ring, &cqe) == 0) {
        // [결과물] cqe->res에 "실제로 몇 바이트를 읽었는지" 숫자가 들어있습니다.
        int bytes_read = cqe->res;

        if (bytes_read > 0) {
            // [데이터 위치] 이미 session_buffer에 데이터가 들어와 있습니다!
            // 우리는 그냥 session_buffer를 읽기만 하면 됩니다.
            printf("받은 데이터: %s\n", session_buffer);
        }

        io_uring_cqe_seen(ring, cqe);
    }
}