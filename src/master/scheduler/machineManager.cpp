//
// Created by he on 1/24/18.
//

#include "machineManager.h"

machineManager::machineManager() {
    m_logicMachineNum = -1;
}

bool machineManager::addOnePhysicsMachine(int machinId, int sockfd, struct sockaddr_in client_address) {
    Machine * machine = new Machine(machinId);
    machine->initSocket(sockfd, client_address);
    m_physicsMachines.push_back(machine);
    updateLogicMap();
    return true;
}

bool machineManager::removeOnePhysicsMachine(int machinId) {
    // TODO
    return false;
}

void machineManager::setLogicMachineNum(int num) {
    m_logicMachineNum = num;
}

void machineManager::updateLogicMap() {
    int phyMachineNum = static_cast<int>(m_physicsMachines.size());
    for (int i = 0; i < m_logicMachineNum; ++i) {
        m_logicMap[i] = m_physicsMachines.at(i % phyMachineNum);
    }
}

bool machineManager::sendTask(int mapperID, char *ins, int inslen) {
    Machine* m = m_logicMap[mapperID];
    return m->setSendMsg(ins, inslen);
}

Machine *machineManager::getPhyMachineByMachineID(int machineID) {
    for (auto &it : m_physicsMachines){
        if (it->getMachineID() == machineID){
            return it;
        }
    }
    return nullptr;
}

int machineManager::getPhysicsMachineNum() {
    return m_physicsMachines.size();
}
