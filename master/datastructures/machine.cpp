//
// Created by he on 1/20/18.
//

#include "machine.h"


Machine::Machine(int machineID) {
    setMachineID(machineID);
}

int Machine::getMachineID() const {
    return machineID;
}

void Machine::setMachineID(int machineID) {
    Machine::machineID = machineID;
}


