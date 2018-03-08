//
// Created by he on 1/20/18.
//

#ifndef MASTER_FLOW_H
#define MASTER_FLOW_H

#define FLOWNOEND 0
#define FLOWEND 1

class Flow {
public:
    Flow(int flowID, int mapperID, int reducerID, double flowSizeMB):flowID(flowID),mapperID(mapperID),reducerID(reducerID),flowSizeMB(flowSizeMB){
        this->remainMB = flowSizeMB;
        this->currentMbs = 0;
        flowState = FLOWNOEND;
        endTime = 0;
    }

    const int getMapperID() const;

    const int getReducerID() const;

    const double getFlowSizeMB() const;

    double getRemainMB() const;

    void setRemainMB(double remainMB);

    double getCurrentMbs() const;

    void setCurrentMbs(double currentMbs);
    int getEndTime() const;
    void setEndTime(int endTime);
    const int getFlowID() const;
    void toString();
private:

    const int flowID;
    const int mapperID;
    const int reducerID;
    const double flowSizeMB;
    double remainMB;
    double currentMbs;
    bool flowState;
    int endTime;
public:
    bool isFlowState() const;
    void setFlowState(bool flowState);

};


#endif //MASTER_FLOW_H
