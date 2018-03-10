cmake_minimum_required(VERSION 3.5)

project(CoflowSim)

set(CMAKE_CXX_STANDARD 11)

# add_subdirectory(thirdparty)
add_subdirectory(src/common)
add_subdirectory(src/master)
add_subdirectory(src/client)
