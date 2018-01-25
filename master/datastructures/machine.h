//
// Created by he on 1/20/18.
//

#ifndef MASTER_MACHINE_H
#define MASTER_MACHINE_H


#include "../socket/socketManage.h"

class Machine : public SocketManage {
public:
    Machine(int machineID):machineID(machineID){}

    int getMachineID() const;

    void run() override;

private:
    const int machineID;
};


#endif //MASTER_MACHINE_H
