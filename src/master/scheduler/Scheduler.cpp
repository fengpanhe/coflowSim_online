//
// Created by he on 1/23/18.
//

#include <sstream>
#include <cstring>
#include <spdlog/spdlog.h>
#include <iostream>
#include <fstream>
#include "Scheduler.h"

void Scheduler::run() {
//  time_t now = time(0);
  startTime = time(0);

  ofstream outfile;
  outfile.open("coflow.log", ios::out | ios::trunc);
  bool flag = false;
  while (true) {
    int delay = 100000000;
    while (--delay > 0);
    if (machines->getPhysicsMachineNum() < 1) {
      continue;
    }
//        printf("de\n");
    registerCoflow(time(0));
    for (int i = 0; i < registerIndex; ++i) {
      // TODO 选出可以发送的coflow，改状态为RUNNING
      if (sCoflows->at(i)->getCoflowState()==REGISTED)
        sCoflows->at(i)->setCoflowState(RUNNING);
    }

    for (int i = 0; i < registerIndex; ++i) {
      Coflow *co = sCoflows->at(i);
      if (co->getCoflowState()==RUNNING) {
        for (auto &it:co->flowCollection) {
          while (!machines->sendTask(co->getCoflowID(),
                                     it->getFlowID(),
                                     it->getMapperID(),
                                     it->getReducerID(),
                                     it->getFlowSizeMB(),
                                     it->getCurrentMbs()));
        }
        co->setCoflowState(RUNNINGED);
      }
    }

    for (auto &it:machines->m_physicsMachines) {
      if (!it->recvMsg()) {
        continue;
      }
//      if (it->getM_recvIdx() <= 0) {
//        continue;
//      }
//
//      int buflen = 0;
//      it->getRecvBuf(recvbuf, buflen);
      it->parseFlowsFinishedInfo();

      int coflowID, flowID, endtime;
      while (it->getOneFlowEndInfo(coflowID, flowID, endtime)) {
//        cout << coflowID << " " << flowID << " " << endtime << endl;
        for (int i = 0; i < sCoflows->size(); i++) {
          if (sCoflows->at(i)->getCoflowID()==coflowID) {
            if (sCoflows->at(i)->flowEnd(flowID, endtime)) {
              cout << "coflow " << coflowID << " end, time " << time(0) - startTime << endl;
              outfile << coflowID << " " << sCoflows->at(i)->getStartTime() << " " << time(0) - startTime<< endl;
            }
            break;
          }
        }
      }
//      stringstream ss;
//      ss << recvbuf;
//      ss >> coflowID;
//      ss >> flowID;
//      ss >> endtime;
//      ss.clear();


    }
//        for (int i = 0; i < sCoflows->size(); ++i) {
//            Coflow* co = sCoflows->at(i);
//            for(auto &it:co->flowCollection){
//                delay = 1000000000;
//                while(--delay > 0);
//                stringstream ss;
//                ss << "coflowID:";
//                ss << co->getCoflowID();
//                ss << "||flow:";
//                ss << it->getMapperID();
//                ss << "->";
//                ss << it->getReducerID();
//                ss << "||";
//                ss << it->getFlowSizeMB();
//                char ch[100];
//                ss >> ch;
//                machines->sendTask(it->getMapperID(), ch, strlen(ch));
//            }
//        }
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
  static auto registerCoflow_console = spdlog::stdout_color_mt("registerCoflow");

//  int nowTime = time(0);
  long t = (currentTime - startTime) * 1000;
  int CoflowNum = sCoflows->size();

  while (true) {
    if (registerIndex >= CoflowNum)
      return;
    if (sCoflows->at(registerIndex)->getStartTime() <= t) {
      sCoflows->at(registerIndex)->setCoflowState(REGISTED);
//      registerCoflow_console->info("{} ,coflow {} registed!", t, sCoflows->at(registerIndex)->getCoflowID());
      registerIndex++;
    } else {
      break;
    }
  }
}
