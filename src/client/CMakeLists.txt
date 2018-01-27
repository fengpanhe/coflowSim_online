# cmake_minimum_required(VERSION 3.9)
# project(client)

# set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/common)

link_directories(${PROJECT_SOURCE_DIR}/common)

add_executable(client main.cpp)

# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

find_package(Threads REQUIRED)
target_link_libraries(client Threads::Threads common)