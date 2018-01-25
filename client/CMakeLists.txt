cmake_minimum_required(VERSION 3.9)
project(client)

set(CMAKE_CXX_STANDARD 11)

add_executable(client main.cpp)

find_package(Threads REQUIRED)
target_link_libraries(client Threads::Threads /home/he/git/coflowSim_online/share/cmake-build-debug/libshare.so)
include_directories(../share)