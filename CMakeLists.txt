cmake_minimum_required(VERSION 4.1)
project(MiniNetTalk)

set(CMAKE_CXX_STANDARD 26)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)