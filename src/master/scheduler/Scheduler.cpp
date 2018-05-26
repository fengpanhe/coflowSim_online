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

  while (!this->run_stop) {
    this->registerCoflow(clock() / TIME_CLOCK);
    this->admitCoflow();
    this->generateTask(clock() / TIME_CLOCK);


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
              outfile << coflowID << " " << sCoflows->at(i)->getRegisterTime()
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
  int CoflowNum = static_cast<int>(sCoflows->size());

  while (true) {
    if (registerIndex >= CoflowNum)
      return;
    Coflow *co = sCoflows->at(static_cast<unsigned long>(registerIndex));
    if (co->getRegisterTime() <= t) {
      co->setCoflowState(REGISTED);
      printf("%ld,coflow %d is registered!\n", t, co->getCoflowID());
      registerIndex++;
    } else {
      break;
    }
  }
}



void Scheduler::admitCoflow() {
  Coflow * min_cct_co = nullptr;
  double min_cct = -1;

  Coflow *co;
  double cct;
  for (int i = 0; i < registerIndex; ++i) {
    // 选出可以预测时间最短的coflow
    co = sCoflows->at(i);
    if (co->getCoflowState() == REGISTED) {
      cct = this->predictCCT(co);
      if(cct != -1) {
        if(min_cct == -1) min_cct = cct;
        if (cct <= min_cct) {
          min_cct = cct;
          min_cct_co = co;
        }
      }
    }
  }
//  printf("\n");

  if(min_cct_co == nullptr || min_cct == LONG_MAX){
    return;
  }
  Flow *f;
  for (FLOWS_MAP_TYPE_IT it = min_cct_co->flowsBegin(); it != min_cct_co->flowsEnd();
       it++) {
    f = it->second;
    f->setCurrentMbs(f->getFlowSizeMB() / min_cct);
//    f->setCurrentMbs(100);
  }
  min_cct_co->setCoflowState(RUNNING);

}

void Scheduler::generateTask(long currentTime) {
  for (int i = 0; i < registerIndex; ++i) {
    Coflow *co = sCoflows->at(i);
    if (co->getCoflowState() == RUNNING) {
      co->setCoflowState(RUNNINGED);
      for (FLOWS_MAP_TYPE_IT it = co->flowsBegin(); it != co->flowsEnd();
           it++) {
        Flow *f = it->second;
        while (!machines->sendTask(co->getCoflowID(), f->getFlowID(),
                                   f->getMapperID(), f->getReducerID(),
                                   f->getFlowSizeMB(), f->getCurrentMbs()))
          ;
        printf("%d %d %lf %lf\n", co->getCoflowID(), f->getFlowID(), f->getFlowSizeMB(), f->getCurrentMbs());
      }
      co->setStartTime(currentTime);
      printf("coflow %d is RUNNING!\n", co->getCoflowID());
    }
  }
}

double Scheduler::predictCCT(Coflow *co) {
  unordered_map<int, double> mapper_and_flows_size;
  unordered_map<int, double>::iterator mf_it;
  Flow *f;
  int machine_id;
  for (FLOWS_MAP_TYPE_IT it = co->flowsBegin(); it != co->flowsEnd();
       it++) {
    f = it->second;
    machine_id = machines->m_logicMap[f->getMapperID()]->getMachineID();
    mf_it = mapper_and_flows_size.find(machine_id);
    if (mf_it == mapper_and_flows_size.end()) {
      mapper_and_flows_size[machine_id] = f->getFlowSizeMB();
    } else{
      mapper_and_flows_size[machine_id] += f->getFlowSizeMB();
    }
  }
  double max_cct = -1;
  double cct;
  for(auto &it : machines->m_physicsMachines){
    mf_it = mapper_and_flows_size.find(it->getMachineID());
    if (mf_it != mapper_and_flows_size.end()){
      if(it->getRemainBandwidth() <= 0){
        cct = -1;
        return cct;
      } else{
        cct = mf_it->second / it->getRemainBandwidth();
        if(cct > max_cct){
          max_cct = cct;
        }
      }
    }
  }
  return max_cct;
}
