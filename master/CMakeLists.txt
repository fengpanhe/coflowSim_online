cmake_minimum_required(VERSION 3.9)
project(master)

set(CMAKE_CXX_STANDARD 11)

add_executable(master main.cpp datastructures/flow.cpp datastructures/flow.h datastructures/coflow.cpp datastructures/coflow.h datastructures/coflowCollection.cpp datastructures/coflowCollection.h datastructures/machine.cpp datastructures/machine.h datastructures/mapper.cpp datastructures/mapper.h datastructures/reducer.cpp datastructures/reducer.h traceProducer/TraceProducer.cpp traceProducer/TraceProducer.h)