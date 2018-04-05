//
// Created by he on 1/20/18.
//

#include "machine.h"
#include <arpa/inet.h>
#include <cstring>
#include <sstream>

int Machine::getMachineID() const { return machineID; }

void Machine::run() { SocketManage::run(); }

bool Machine::createConnect() {
  SocketManage::createConnect(this->socketip, this->socketport);
}

int Machine::getRemainBandwidth() const { return remainBandwidth; }

void Machine::setRemainBandwidth(int remainBandwidth) {
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
      ss >> flowEndInfo->endTime;
      ss.clear();
      flowsFinishedInfo.push(flowEndInfo);
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
