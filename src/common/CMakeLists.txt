# cmake_minimum_required(VERSION 3.9)
# project(common)

# set(CMAKE_CXX_STANDARD 11)


# 设置输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)

aux_source_directory(. DIR_SRCS_)
aux_source_directory(./socket DIR_SRCS_SOCKET)
aux_source_directory(./lib DIR_SRCS_LIB)
aux_source_directory(./webserver DIR_SRCS_WEBSERVER)
#add_library(common SHARED ${DIR_SRCS_} ${DIR_SRCS_SOCKET} ${DIR_SRCS_LIB})
add_library(common STATIC ${DIR_SRCS_} ${DIR_SRCS_SOCKET} ${DIR_SRCS_LIB} ${DIR_SRCS_WEBSERVER})