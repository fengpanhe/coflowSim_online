//
// Created by he on 1/23/18.
//

#ifndef MASTER_SCHEDULER_H
#define MASTER_SCHEDULER_H

#include <queue>
#include "../datastructures/coflow.h"
#include "../datastructures/machine.h"
#include "lib/threadclass.h"
#include "MachineManager.h"

class Scheduler : public ThreadClass {
public:
  Scheduler() {
    registerIndex = 0;

    recvbuf = new char[1024];
  }

  ~Scheduler(){
    delete recvbuf;
  }

  void run() override;

  bool onCoflowRegister(Coflow *c);

  bool onCoflowUnregister(Coflow *c);

  bool setCoflows(vector<Coflow *> *coflows);

  // 模拟coflow的注册过程
  void registerCoflow(long currentTime);
private:
  vector<Coflow *> *sCoflows;
  int registerIndex;

  vector<Coflow *> *registedCoflows;
  vector<Coflow *> *mRunningCoflows;
  vector<Coflow *> *mFinishedCoflows;
  vector<Coflow *> *mNotAdmittedCoflows;

  //    TODO
  MachineManager *machines;

  long startTime;

  char * recvbuf;
public:
  void setMachines(MachineManager *machines);
};

#endif // MASTER_SCHEDULER_H
