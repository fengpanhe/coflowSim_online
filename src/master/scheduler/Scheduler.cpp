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
    // int delay = 100000000;
    // while (--delay > 0)
    //   ;
    if (machines->getPhysicsMachineNum() < 1) {
      continue;
    }
    //        printf("de\n");
    registerCoflow(clock() / TIME_CLOCK);
    for (int i = 0; i < registerIndex; ++i) {
      // TODO 选出可以发送的coflow，改状态为RUNNING
      if (sCoflows->at(i)->getCoflowState() == REGISTED)
        sCoflows->at(i)->setCoflowState(RUNNING);
    }

    for (int i = 0; i < registerIndex; ++i) {
      Coflow *co = sCoflows->at(i);
      if (co->getCoflowState() == RUNNING) {
        for (auto &it : co->flowCollection) {
          while (!machines->sendTask(co->getCoflowID(), it->getFlowID(),
                                     it->getMapperID(), it->getReducerID(),
                                     it->getFlowSizeMB(), it->getCurrentMbs()))
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
        //        cout << coflowID << " " << flowID << " " << endtime << endl;
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

  //  int nowTime = time(0);
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
