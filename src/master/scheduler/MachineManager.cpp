//
// Created by he on 1/24/18.
//

#include "MachineManager.h"

MachineManager::MachineManager() {
    m_logicMachineNum = -1;
}

bool MachineManager::addOnePhysicsMachine(int machinId, int sockfd, struct sockaddr_in client_address) {
    Machine * machine = new Machine(machinId);
    machine->initSocket(sockfd, client_address);
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

bool MachineManager::sendTask(int mapperID, char *ins, int inslen) {
    Machine* m = m_logicMap[mapperID];
    return m->setSendMsg(ins, inslen);
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
