//
// Created by he on 1/23/18.
//

#include "Scheduler.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>
#define TIME_CLOCK 1000

void Scheduler::run() {
  //  time_t now = time(0);
  startTime = clock() / TIME_CLOCK;

  ofstream outfile;
  outfile.open("coflow.log", ios::out | ios::trunc);
  bool flag = false;
  int coflowFinishedNum = 0;

  while (true) {
    registerCoflow(clock() / TIME_CLOCK);

    for (int i = 0; i < registerIndex; ++i) {
      // TODO 选出可以发送的coflow，改状态为RUNNING
      Coflow * co = sCoflows->at(i);
      if (co->getCoflowState() == REGISTED){
        co->setCoflowState(RUNNING);
      }

    }

    for (int i = 0; i < registerIndex; ++i) {
      Coflow *co = sCoflows->at(i);
      if (co->getCoflowState() == RUNNING) {
        for (FLOWS_MAP_TYPE_IT it = co->flowsBegin(); it != co->flowsEnd();
             it++) {
          Flow *f = it->second;
          f->setCurrentMbs(10);
          while (!machines->sendTask(co->getCoflowID(), f->getFlowID(),
                                     f->getMapperID(), f->getReducerID(),
                                     f->getFlowSizeMB(), f->getCurrentMbs()))
            ;
        }
        co->setCoflowState(RUNNINGED);
      }
    }

    for (auto &it : machines->m_physicsMachines) {
      if (!it->recvMsg()) {
        continue;
      }
      it->parseFlowsFinishedInfo();
      int coflowID, flowID, endtime;
      while (it->getOneFlowEndInfo(coflowID, flowID, endtime)) {
        for (int i = 0; i < sCoflows->size(); i++) {
          if (sCoflows->at(i)->getCoflowID() == coflowID) {
            if (sCoflows->at(i)->flowEnd(flowID, endtime)) {
              long ctime = clock() / TIME_CLOCK;
              cout << "coflow " << coflowID << " end, time "
                   << ctime - startTime << endl;
              outfile << coflowID << " " << sCoflows->at(i)->getStartTime()
                      << " " << ctime - startTime << endl;
              coflowFinishedNum++;
              if (coflowFinishedNum >= sCoflows->size()) {
                cout << "全部完成！" << endl;
                for (auto &ma : machines->m_physicsMachines) {
                  ma->closeConn();
                }
                return;
              }
            }
            break;
          }
        }
      }
    }
    if (flag)
      break;
  }
  outfile.close();
}

void Scheduler::setMachines(MachineManager *machines) {
  Scheduler::machines = machines;
}

bool Scheduler::setCoflows(vector<Coflow *> *coflows) {
  sCoflows = coflows;
  return true;
}
void Scheduler::registerCoflow(long currentTime) {
  static auto registerCoflow_console =
      spdlog::stdout_color_mt("registerCoflow");

  long t = currentTime - startTime;
  int CoflowNum = sCoflows->size();

  while (true) {
    if (registerIndex >= CoflowNum)
      return;
    if (sCoflows->at(registerIndex)->getStartTime() <= t) {
      sCoflows->at(registerIndex)->setCoflowState(REGISTED);
      cout << t << ", coflow " << sCoflows->at(registerIndex)->getCoflowID()
           << " is registered!" << endl;
      registerIndex++;
    } else {
      break;
    }
  }
}
