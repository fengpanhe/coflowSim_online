//
// Created by he on 1/24/18.
//

#include <sstream>
#include <cstring>
#include <iostream>
#include "MachineManager.h"

MachineManager::MachineManager() {
    m_logicMachineNum = -1;
}

bool MachineManager::addOnePhysicsMachine(int machinId, char* sockip, int sockport, struct sockaddr_in connAddr) {
    Machine * machine = new Machine(machinId, sockip, sockport);
    machine->initSocket(machinId, connAddr);
    m_physicsMachines.push_back(machine);
    updateLogicMap();
    return true;
}

bool MachineManager::removeOnePhysicsMachine(int machinId) {
    // TODO
    return false;
}

void MachineManager::setLogicMachineNum(int num) {
    m_logicMachineNum = num;
}

void MachineManager::updateLogicMap() {
    int phyMachineNum = static_cast<int>(m_physicsMachines.size());
    for (int i = 0; i < m_logicMachineNum; ++i) {
        m_logicMap[i] = m_physicsMachines.at(i % phyMachineNum);
    }
}

bool MachineManager::sendTask(int coflowID, int flowID, int mapperID, int reducerID, double flowSizeMB, double sendSpeedMbs) {
    Machine* m = m_logicMap[mapperID];
    Machine* r = m_logicMap[reducerID];
    char ch[100] = "a";
    stringstream ss;

    ss << coflowID;
    ss >> ch;
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
    cout << ch << endl;
    return m->setSendMsg(ch, strlen(ch));
}

Machine *MachineManager::getPhyMachineByMachineID(int machineID) {
    for (auto &it : m_physicsMachines){
        if (it->getMachineID() == machineID){
            return it;
        }
    }
    return nullptr;
}

int MachineManager::getPhysicsMachineNum() {
    return m_physicsMachines.size();
}
