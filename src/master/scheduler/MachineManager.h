//
// Created by he on 1/24/18.
//

#ifndef MASTER_MACHINEMANAGER_H
#define MASTER_MACHINEMANAGER_H

#include "../datastructures/machine.h"
#include <unordered_map>
#include <vector>
using namespace std;

class MachineManager {
public:
  MachineManager();

  // 返回machineID
  int addOnePhysicsMachine(char *sockip, int sockport, double bandwidth);
  bool startConn();
  Machine *getPhyMachineByMachineID(int machineID);

  bool removeOnePhysicsMachine(int machinId);

  int getPhysicsMachineNum();

  void setLogicMachineNum(int num);

  void updateLogicMap();

  bool sendTask(int coflowID, int flowID, int mapperID, int reducerID,
                double flowSizeMB, double sendSpeedMbs);

  // 逻辑机到物理机的映射
  unordered_map<int, Machine *> m_logicMap;
  vector<Machine *> m_physicsMachines;

private:
  int m_logicMachineNum;
  int m_physicsMachinesNum;
};

#endif // MASTER_MACHINEMANAGER_H
