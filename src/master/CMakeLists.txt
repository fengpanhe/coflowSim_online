# cmake_minimum_required(VERSION 3.9)
# project(master)

# set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/common)

link_directories(${PROJECT_SOURCE_DIR}/common)

add_executable(master main.cpp datastructures/flow.cpp datastructures/flow.h datastructures/coflow.cpp datastructures/coflow.h datastructures/coflowCollection.cpp datastructures/coflowCollection.h datastructures/machine.cpp datastructures/machine.h traceProducer/producer.cpp traceProducer/producer.h traceProducer/CoflowBenchmarkTraceProducer.cpp traceProducer/CoflowBenchmarkTraceProducer.h  scheduler/scheduler.cpp scheduler/scheduler.h scheduler/coflowManager.cpp scheduler/coflowManager.h  scheduler/machineManager.cpp scheduler/machineManager.h)

# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

find_package(Threads REQUIRED)
target_link_libraries(master Threads::Threads common)