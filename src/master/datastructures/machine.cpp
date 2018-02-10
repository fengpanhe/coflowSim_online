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




