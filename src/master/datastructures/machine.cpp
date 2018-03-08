//
// Created by he on 1/20/18.
//

#include "machine.h"




int Machine::getMachineID() const {
    return machineID;
}

void Machine::run() {
    SocketManage::run();
}
int Machine::getRemainBandwidth() const
{
    return remainBandwidth;
}
void Machine::setRemainBandwidth(int remainBandwidth)
{
    Machine::remainBandwidth = remainBandwidth;
}

char* Machine::getSocketip() const
{
    return socketip;
}
void Machine::setSocketip(char* socketip)
{
    Machine::socketip = socketip;
}
int Machine::getSocketport() const
{
    return socketport;
}
void Machine::setSocketport(int socketport)
{
    Machine::socketport = socketport;
}




