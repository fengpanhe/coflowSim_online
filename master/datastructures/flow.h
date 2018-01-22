//
// Created by he on 1/20/18.
//

#ifndef MASTER_FLOW_H
#define MASTER_FLOW_H


#include "mapper.h"
#include "reducer.h"

class Flow {
public:
    Flow(int mapperID, int reducerID, double flowSizeMB):mapperID(mapperID),reducerID(reducerID),flowSizeMB(flowSizeMB){
        this->remainMB = flowSizeMB;
        this->currentMbs = 0;
    }

    const int getMapperID() const;

    const int getReducerID() const;

    const double getFlowSizeMB() const;

    double getRemainMB() const;

    void setRemainMB(double remainMB);

    double getCurrentMbs() const;

    void setCurrentMbs(double currentMbs);

    void toString();
private:
    const int mapperID;
    const int reducerID;
    const double flowSizeMB;
    double remainMB;
    double currentMbs;

};


#endif //MASTER_FLOW_H
