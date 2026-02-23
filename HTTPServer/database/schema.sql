-- 유저 기본 정보 테이블
CREATE TABLE users (
    uid INT AUTO_INCREMENT PRIMARY KEY,
    login_id VARCHAR(16) NOT NULL UNIQUE,
    password CHAR(60) NOT NULL,
    inventory JSON DEFAULT ('{}'),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);