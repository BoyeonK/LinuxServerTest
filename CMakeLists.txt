cmake_minimum_required(VERSION 3.16)
project(LinuxServerTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(URING_LIB uring REQUIRED)

set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)

# FetchContent 모듈 불러오기
include(FetchContent)

# 내 라이브러리 선언
FetchContent_Declare(
    MyUtils
    GIT_REPOSITORY https://github.com/BoyeonK/myUtils.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(MyUtils)

set(PROJECT_SOURCES
    src/main.cpp
    src/TestCode/NetAddress.cpp
    src/IPCSocketWrapper.cpp
    src/IOTask.cpp
    src/HTTPserver.cpp
    src/IoUringWrapper.cpp
    src/ObjectPool.cpp
    src/SendBuffer.cpp
    src/S2HPacketHandler.cpp
    src/RecvBuffer.cpp

    src/TestCode/NetAddress.h
    src/IPCSocketWrapper.h
    src/IOTask.h
    src/HTTPserver.h
    src/IoUringWrapper.h
    src/ObjectPool.h
    src/SendBuffer.h
    src/S2HPacketHandler.h
    src/S2HPacketMaker.h
    src/RecvBuffer.h
)

# 내 서버 실행 파일 생성
add_executable(LinuxServerTest ${PROJECT_SOURCES})

target_include_directories(LinuxServerTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(LinuxServerTest PRIVATE 
    MyUtils::MyUtils 
    ${URING_LIB}
    ${Protobuf_LIBRARIES}
)