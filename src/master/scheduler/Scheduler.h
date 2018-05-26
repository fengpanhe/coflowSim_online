//
// Created by he on 1/23/18.
//

#ifndef MASTER_SCHEDULER_H
#define MASTER_SCHEDULER_H

#include "../datastructures/coflow.h"
#include "../datastructures/machine.h"
#include "MachineManager.h"
#include "lib/ThreadPool.h"
#include <queue>
#include <rapidjson/document.h>

class Scheduler : public ThreadClass {
public:
  Scheduler() {
    registerIndex = 0;

    recvbuf = new char[1024];
  }

  ~Scheduler() { delete recvbuf; }

  void run() override;

  bool onCoflowRegister(Coflow *c);

  bool onCoflowUnregister(Coflow *c);

  bool setCoflows(vector<Coflow *> *coflows);
  void setMachines(MachineManager *machines);

  // coflow的注册
  void registerCoflow(long currentTime);
  // 判断那些coflow可以运行
  void admitCoflow();
  // 将可以运行的coflow生成任务
  void generateTask(long currentTime);

private:
  vector<Coflow *> *sCoflows;
  int registerIndex;

  double predictCCT(Coflow * co);
  // vector<Coflow *> *registedCoflows;
  // vector<Coflow *> *mRunningCoflows;
  // vector<Coflow *> *mFinishedCoflows;
  // vector<Coflow *> *mNotAdmittedCoflows;

  //    TODO
  MachineManager *machines;

  long startTime;

  char *recvbuf;
};

#endif // MASTER_SCHEDULER_H
