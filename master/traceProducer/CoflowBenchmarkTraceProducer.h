//
// Created by he on 1/21/18.
//

#ifndef MASTER_COFLOWBENCHMARKTRACEPRODUCER_H
#define MASTER_COFLOWBENCHMARKTRACEPRODUCER_H


#include <string>
#include "producer.h"
#include "../datastructures/coflowCollection.h"

class CoflowBenchmarkTraceProducer: public Producer {
public:
    CoflowBenchmarkTraceProducer(string coflowBenchFilePath, string machineDefineFilePath);

    CoflowCollection* prepareCoflows() override;

    vector<Machine*>* prepareMachines() override;

    int getCoflowNum() override;

    int getLogicMachinePortNum();

private:
    string coflowBenchFilePath;
    string machineDefineFilePath;
    int logicMachinePortNum;
    int coflowNum;

    Coflow* prepareOneCoflow(char* line);
};


#endif //MASTER_COFLOWBENCHMARKTRACEPRODUCER_H
