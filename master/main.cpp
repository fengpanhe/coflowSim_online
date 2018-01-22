#include <iostream>
#include "traceProducer/producer.h"
#include "traceProducer/CoflowBenchmarkTraceProducer.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    CoflowBenchmarkTraceProducer producer("../../FB2010-1Hr-150-0.txt", "123");
    CoflowCollection* coflows = producer.prepareCoflows();
    coflows->getCoflowByindex(525)->toString();
    return 0;
}