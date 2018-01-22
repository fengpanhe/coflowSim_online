//
// Created by he on 1/20/18.
//

#ifndef MASTER_MACHINE_H
#define MASTER_MACHINE_H


class Machine {
public:
    Machine(int machineID, char * machineIP, int machinePORT):machineID(machineID),machineIP(machineIP),machinePORT(machinePORT);

    int getMachineID() const;
    void setMachineID(int machineID);
private:/
    const int machineID;
    const char * machineIP;
    const int machinePORT;
};


#endif //MASTER_MACHINE_H
