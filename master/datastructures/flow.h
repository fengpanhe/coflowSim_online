//
// Created by he on 1/20/18.
//

#ifndef MASTER_FLOW_H
#define MASTER_FLOW_H


#include "mapper.h"
#include "reducer.h"

class Flow {
public:
    Flow(Mapper* mapper, Reducer* reducer, double flowSizeMB):mapper(mapper),reducer(reducer),flowSizeMB(flowSizeMB){
        this->remainMB = flowSizeMB;
    }

    const Mapper *getMapper() const {
        return mapper;
    }

    const Reducer *getReducer() const {
        return reducer;
    }

    const double getFlowSizeMB() const {
        return flowSizeMB;
    }

    double getRemainMB() const {
        return remainMB;
    }

    double getCurrentMbs() const {
        return currentMbs;
    }

    void setRemainMB(double remainMB) {
        Flow::remainMB = remainMB;
    }

    void setCurrentMbs(double currentMbs) {
        Flow::currentMbs = currentMbs;
    }

private:
    const Mapper* mapper;
    const Reducer* reducer;
    const double flowSizeMB;
    double remainMB;
    double currentMbs;

};


#endif //MASTER_FLOW_H
