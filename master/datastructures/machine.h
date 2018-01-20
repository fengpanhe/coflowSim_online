//
// Created by he on 1/20/18.
//

#ifndef MASTER_MACHINE_H
#define MASTER_MACHINE_H


class Machine {
public:
    Machine(int machineID);

    int getMachineID() const;
    void setMachineID(int machineID);
private:
    int machineID;
};


#endif //MASTER_MACHINE_H
