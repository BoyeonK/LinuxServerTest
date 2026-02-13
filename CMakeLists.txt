cmake_minimum_required(VERSION 3.16)
project(LinuxServerTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(URING_LIB uring)

# FetchContent 모듈 불러오기
include(FetchContent)

# 내 라이브러리 선언
FetchContent_Declare(
    MyUtils
    GIT_REPOSITORY https://github.com/BoyeonK/myUtils.git
    GIT_TAG        main
)
# 내 라이브러리 다운로드 및 설정 (자동으로 add_subdirectory 함)
FetchContent_MakeAvailable(MyUtils)

set(PROJECT_SOURCES
    src/main.cpp
    src/TestCode/MultiThreadTest.cpp
    src/TestCode/NetAddress.cpp
    src/TestCode/LobbySession.cpp
    src/TestCode/LobbyServerService.cpp

    src/TestCode/MultiThreadTest.h 
    src/TestCode/NetAddress.h
    src/TestCode/LobbySession.h
    src/TestCode/LobbyServerService.h
)

# 내 게임 실행 파일 생성
add_executable(LinuxServerTest ${PROJECT_SOURCES})

target_include_directories(LinuxServerTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 라이브러리 링크 (아까 만든 별칭 사용)
target_link_libraries(LinuxServerTest PRIVATE MyUtils::MyUtils ${URING_LIB})