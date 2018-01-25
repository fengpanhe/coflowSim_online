//
// Created by he on 1/24/18.
//

#ifndef MASTER_MACHINEMANAGER_H
#define MASTER_MACHINEMANAGER_H

#include <vector>
#include <unordered_map>
#include "../datastructures/machine.h"
using namespace std;

class machineManager {
public:
    machineManager();

    bool addOnePhysicsMachine(int machinId, int sockfd, struct sockaddr_in client_address);

    Machine* getPhyMachineByMachineID(int machineID);

    bool removeOnePhysicsMachine(int machinId);

    void setLogicMachineNum(int num);

    void updateLogicMap();

    bool sendTask(int mapperID, char * ins, int inslen);

    // 逻辑机到物理机的映射
    unordered_map<int, Machine*> m_logicMap;
private:
    vector<Machine*> m_physicsMachines;


    int m_logicMachineNum;

};


#endif //MASTER_MACHINEMANAGER_H
