# cmake_minimum_required(VERSION 3.9)
# project(common)

# set(CMAKE_CXX_STANDARD 11)

add_library(common SHARED library.cpp library.h lib/epollFunctions.cpp lib/epollFunctions.h lib/locker.h lib/thread.h lib/threadpool.h socket/sender.cpp socket/sender.h socket/socketManage.cpp socket/socketManage.h)