//
// Created by he on 1/23/18.
//

#ifndef MASTER_SCHEDULER_H
#define MASTER_SCHEDULER_H

#include "../datastructures/coflow.h"
#include "../datastructures/machine.h"
#include "lib/threadclass.h"
#include "MachineManager.h"

class Scheduler : public ThreadClass {
public:
  void run() override;

  bool onCoflowRegister(Coflow *c);

  bool onCoflowUnregister(Coflow *c);

  bool setUnregisterCoflows(vector<Coflow *> *coflows);

private:
  vector<Coflow *> *mUnregisterCoflows;
  vector<Coflow *> *mRunningCoflows;
  vector<Coflow *> *mFinishedCoflows;
  vector<Coflow *> *mNotAdmittedCoflows;

  //    TODO
  MachineManager *machines;

public:
  void setMachines(MachineManager *machines);
};

#endif // MASTER_SCHEDULER_H
