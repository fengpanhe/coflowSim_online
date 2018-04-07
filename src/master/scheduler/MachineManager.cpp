//
// Created by he on 1/24/18.
//

#include "MachineManager.h"
#include <cstring>
#include <iostream>
#include <sstream>

MachineManager::MachineManager() {
  m_logicMachineNum = -1;
  m_physicsMachinesNum = 0;
}

int MachineManager::addOnePhysicsMachine(char *sockip, int sockport) {
  Machine *machine = new Machine(m_physicsMachinesNum++, sockip, sockport);
  m_physicsMachines.push_back(machine);
  updateLogicMap();
  return m_physicsMachinesNum - 1;
}

bool MachineManager::startConn() {
  for (auto &it : this->m_physicsMachines) {
    it->createConnect();
  }
}

bool MachineManager::removeOnePhysicsMachine(int machinId) {
  // TODO
  return false;
}

void MachineManager::setLogicMachineNum(int num) { m_logicMachineNum = num; }

void MachineManager::updateLogicMap() {
  int phyMachineNum = static_cast<int>(m_physicsMachines.size());
  for (int i = 0; i < m_logicMachineNum; ++i) {
    m_logicMap[i] = m_physicsMachines.at(i % phyMachineNum);
  }
}

bool MachineManager::sendTask(int coflowID, int flowID, int mapperID,
                              int reducerID, double flowSizeMB,
                              double sendSpeedMbs) {
  Machine *m = m_logicMap[mapperID];
  Machine *r = m_logicMap[reducerID];
  char ch[100] = "a";
  memset(ch, '\0', 100);
  ch[0] = '(';
  stringstream ss;

  ss << coflowID;
  ss >> ch + strlen(ch);
  ss.clear();

  ch[strlen(ch)] = ' ';

  ss << flowID;
  ss >> ch + strlen(ch);
  ss.clear();

  ch[strlen(ch)] = ' ';

  ss << r->getSocketip();
  ss >> ch + strlen(ch);
  ss.clear();

  ch[strlen(ch)] = ' ';

  ss << r->getSocketport();
  ss >> ch + strlen(ch);
  ss.clear();

  ch[strlen(ch)] = ' ';

  ss << coflowID;
  ss << "_";
  ss << flowID;
  ss << "_";
  ss << flowSizeMB;
  ss << "_";
  ss << sendSpeedMbs;
  ss << ".txt";
  ss >> ch + strlen(ch);
  ss.clear();

  ch[strlen(ch)] = ' ';

  ss << flowSizeMB;
  ss >> ch + strlen(ch);
  ss.clear();

  ch[strlen(ch)] = ' ';

  ss << sendSpeedMbs;
  ss >> ch + strlen(ch);
  ss.clear();
  ch[strlen(ch)] = ')';
  //  cout << ch << endl;
  m->setSendMsg(ch, strlen(ch));
  m->sendMsg();
  return true;
}

Machine *MachineManager::getPhyMachineByMachineID(int machineID) {
  for (auto &it : m_physicsMachines) {
    if (it->getMachineID() == machineID) {
      return it;
    }
  }
  return nullptr;
}

int MachineManager::getPhysicsMachineNum() { return m_physicsMachines.size(); }
