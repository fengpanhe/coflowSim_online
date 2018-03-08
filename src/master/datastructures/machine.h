//
// Created by he on 1/20/18.
//

#ifndef MASTER_MACHINE_H
#define MASTER_MACHINE_H

#include <socket/socketManage.h>

class Machine : public SocketManage {
public:
    Machine(int machineID, char* sockip, int sockport)
            :machineID(machineID) {
        remainBandwidth = 0;
        setSocketip(sockip);
        setSocketport(sockport);
    }

    int getMachineID() const;

    void run() override;

    int getRemainBandwidth() const;
    void setRemainBandwidth(int remainBandwidth);

    char* getSocketip() const;
    void setSocketip(char* socketip);
    int getSocketport() const;
    void setSocketport(int socketport);
private:
    const int machineID;
    char* socketip;
    int socketport;
    int remainBandwidth; // 记录本机的剩余带宽

};

#endif // MASTER_MACHINE_H
