//
// Created by he on 1/20/18.
//

#include "machine.h"
#include <arpa/inet.h>
#include <cstring>
#include <sstream>

static auto machine_logger_console = spdlog::stdout_color_mt("machine_logger");

static std::shared_ptr<spdlog::logger> machine_logger_file = NULL;

Machine::Machine(int machineID, char *sockip, int sockport)
    : machineID(machineID) {
  remainBandwidth = 0;
  this->setSocketip(sockip);
  this->setSocketport(sockport);
  tmprecv = new char[TMORECV_MAX_SIZE];
  memset(tmprecv, '\0', TMORECV_MAX_SIZE);
  tmprecvIndex = 0;

  if (machine_logger_file == NULL) {
    try {
      spdlog::set_async_mode(8192);
      machine_logger_file = spdlog::rotating_logger_mt(
          "machine_file_logger", "machine_logger.log", 1024 * 1024 * 5, 3);
      spdlog::drop_all();
    } catch (const spdlog::spdlog_ex &ex) {
      std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
  }
}

int Machine::getMachineID() const { return machineID; }

void Machine::run() { SocketManage::run(); }

bool Machine::createConnect() {
  SocketManage::createConnect(this->socketip, this->socketport);
}

double Machine::getRemainBandwidth() const { return remainBandwidth; }

void Machine::setRemainBandwidth(double remainBandwidth) {
  Machine::remainBandwidth = remainBandwidth;
}

char *Machine::getSocketip() { return socketip; }

void Machine::setSocketip(char *socketip) {
  memset(this->socketip, '\0', 64);
  int silen = strlen(socketip);
  for (int i = 0; i < silen; i++) {
    this->socketip[i] = socketip[i];
  }
}

int Machine::getSocketport() const { return socketport; }

void Machine::setSocketport(int socketport) {
  Machine::socketport = socketport;
}

bool Machine::getOneFlowEndInfo(int &coflowID, int &flowID, int &endtime) {
  if (this->flowsFinishedInfo.size() <= 0) {
    return false;
  }
  FlowEndInfo *flowEndInfo = this->flowsFinishedInfo.front();
  coflowID = flowEndInfo->coflowID;
  flowID = flowEndInfo->flowID;
  endtime = flowEndInfo->endTime;
  delete flowEndInfo;
  this->flowsFinishedInfo.pop();
  return true;
}

bool Machine::parseFlowsFinishedInfo() {
  if (this->getM_recvIdx() <= 0) {
    return false;
  }
  this->getRecvBuf(this->tmprecv, this->tmprecvIndex);
  int flowEndIndex;
  int flowStartIndex;
  for (int i = 0; i < tmprecvIndex; i++) {
    if (tmprecv[i] == '(') {
      flowStartIndex = i + 1;
    }

    if (tmprecv[i] == ')') {
      flowEndIndex = i;
      tmprecv[flowEndIndex] = '\0';
      stringstream ss;
      ss << tmprecv + flowStartIndex;
      FlowEndInfo *flowEndInfo = new FlowEndInfo();
      ss >> flowEndInfo->coflowID;
      ss >> flowEndInfo->flowID;
      ss >> flowEndInfo->speed_Mbs;
      ss >> flowEndInfo->endTime;
      ss.clear();
      flowsFinishedInfo.push(flowEndInfo);
      this->remainBandwidth += flowEndInfo->speed_Mbs;
    }
  }
  if (flowEndIndex < flowStartIndex) {
    int i = 0;
    tmprecv[i++] = '(';
    while (flowStartIndex < tmprecvIndex) {
      tmprecv[i++] = tmprecv[flowStartIndex++];
    }
    tmprecv[i] = '\0';
    tmprecvIndex = i;
  } else {
    tmprecv[0] = '\0';
    tmprecvIndex = 0;
  }
  return true;
}
bool Machine::sendTaskIns(int coflowID, int flowID, char * reducer_ip, int reducer_port, double flowSizeMB, double sendSpeedMbs) {
  char ch[100];
  sprintf(ch, "(%d %d %s %d %d_%d.txt %lf %lf)", coflowID, flowID, reducer_ip, reducer_port, coflowID, flowID, flowSizeMB, sendSpeedMbs);
  this->setSendMsg(ch, static_cast<int>(strlen(ch)));
  this->sendMsg();
  this->remainBandwidth -= sendSpeedMbs;
  return true;
}
