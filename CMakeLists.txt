cmake_minimum_required(VERSION 3.16)
project(LinuxServerTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent 모듈 불러오기
include(FetchContent)

# 라이브러리 선언
FetchContent_Declare(
    MyUtils
    GIT_REPOSITORY https://github.com/BoyeonK/myUtils.git
    GIT_TAG        main
)

# 라이브러리 다운로드 및 설정 (자동으로 add_subdirectory 함)
FetchContent_MakeAvailable(MyUtils)

# 내 게임 실행 파일 생성
add_executable(LinuxServerTest main.cpp)

# 라이브러리 링크 (아까 만든 별칭 사용)
target_link_libraries(LinuxServerTest PRIVATE MyUtils::MyUtils)