cmake_minimum_required(VERSION 4.1)
project(MiniNetTalk)
set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

FetchContent_Declare(
        toml11
        GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
        GIT_TAG v4.4.0
        GIT_SHALLOW    TRUE              # 只拉取最新版本，快！
        EXCLUDE_FROM_ALL                 # 没用到就不编，省时间！
)
FetchContent_Declare(
        beast
        GIT_REPOSITORY https://github.com/boostorg/beast.git
        GIT_TAG boost-1.90.0
        GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(toml11)
#FetchContent_MakeAvailable(beast)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)