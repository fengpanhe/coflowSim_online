//
// Created by he on 1/20/18.
//

#ifndef MASTER_MACHINE_H
#define MASTER_MACHINE_H

#include <socket/socketManage.h>

class Machine : public SocketManage {
public:
    Machine(int machineID)
            :machineID(machineID) {
        remainBandwidth = 0;
    }

    int getMachineID() const;

    void run() override;

    int getRemainBandwidth() const;
    void setRemainBandwidth(int remainBandwidth);

private:
    const int machineID;
    int remainBandwidth; // 记录本机的剩余带宽

};

#endif // MASTER_MACHINE_H
