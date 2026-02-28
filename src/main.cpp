#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <filesystem>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include "GlobalVariable.h"
#include "S2HPacketHandler.h"
#include "IoUringWrapper.h"
#include "IOTask.h"
#include "HTTPserver.h"
#include "SocketWrapper.h"
#include "ObjectPool.h"
#include "RedisHandler.h"
#include "EnvSetter.h"

int main() {
    SetEnv();

    const char* env_redis_host  = std::getenv("REDIS_HOST");
    const char* env_redis_port  = std::getenv("REDIS_PORT");
    const char* env_db_host     = std::getenv("MYSQL_HOST");
    const char* env_db_port     = std::getenv("MYSQL_PORT");
    const char* env_db_user     = std::getenv("MYSQL_USER");
    const char* env_db_pass     = std::getenv("MYSQL_PASSWORD");
    const char* env_db_name     = std::getenv("MYSQL_DATABASE");

    std::string redis_url = "tcp://" + std::string(env_redis_host) + ":" + std::string(env_redis_port);

    std::string mysql_url = "tcp://" + std::string(env_db_host) + ":" + std::string(env_db_port);                               

    try {
        IORing = new IoUringWrapper();
        pRedis = new sw::redis::Redis(redis_url);
        std::cout << "C1 - OK : 글로벌 변수 초기화 완료" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "C1 - X : 글로벌 변수 초기화 실패: " << e.what() << std::endl;
        return 1;
    }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> db_conn(driver->connect(mysql_url, env_db_user, env_db_pass));
        db_conn->setSchema(env_db_name);

        RedisHandler::InitializeItemCache(db_conn.get(), *pRedis);

        std::cout << "C2-3 - OK : MySQL에서 Redis에 items필드 가져오는 중" << std::endl;
    } catch (const sql::SQLException& e) {
        std::cerr << "C2-3 - X : MySQL에서 Redis에 items필드 가져오기 대실패: " << e.what() << std::endl;
        return 1;
    }

    S2HPacketHandler::Init();

    std::unique_ptr<IPCListenSocketWrapper> httpsIpc;
    std::unique_ptr<IPCListenSocketWrapper> dedicateIpc;
    pid_t nodePid;
    try {
        httpsIpc = std::make_unique<IPCListenSocketWrapper>("/tmp/https.sock", 1);
        httpsIpc->Init();

        dedicateIpc = std::make_unique<IPCListenSocketWrapper>("/tmp/dedicate.sock", 128);
        dedicateIpc->Init();

        if (launchNode(nodePid))
            std::cout << "C3-2 - OK : 자식 프로세스로 HTTP서버 구동 (PID: " << nodePid << ")" << std::endl;

        H2SAcceptTask* httpAcceptTask = ObjectPool<H2SAcceptTask>::Acquire(
            httpsIpc->GetFd(), 
            IORing
        );
        IORing->RegisterAcceptTask(httpsIpc->GetFd(), httpAcceptTask);
        
    } catch (const std::exception& e) {
        std::cerr << "C3-2 - X : 소켓 생성 ~ 초기화 실패 : " << e.what() << std::endl;
        _exit(1);
    }

    while (true) {
        IORing->ExecuteCQTask();

        std::this_thread::yield();
    }

    return 0;
}