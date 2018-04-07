//
// Created by he on 1/21/18.
//

#ifndef MASTER_COFLOWBENCHMARKTRACEPRODUCER_H
#define MASTER_COFLOWBENCHMARKTRACEPRODUCER_H

#include "../datastructures/coflowCollection.h"
#include "producer.h"
#include <string>

class CoflowBenchmarkTraceProducer : public Producer {
public:
  CoflowBenchmarkTraceProducer(string coflowBenchFilePath,
                               string machineDefineFilePath);

  bool prepareCoflows(vector<Coflow *> *&coflows) override;

  vector<Machine *> *prepareMachines() override;

  int getCoflowNum() override;

  int getLogicMachinePortNum();

private:
  string coflowBenchFilePath;
  string machineDefineFilePath;
  int logicMachinePortNum;
  int coflowNum;

  Coflow *prepareOneCoflow(char *line);
};

#endif // MASTER_COFLOWBENCHMARKTRACEPRODUCER_H
