//
// Created by he on 1/20/18.
//

#ifndef MASTER_COFLOW_H
#define MASTER_COFLOW_H

#define UNREGISTER 0
#define REGISTED   1
#define RUNNING 2
#define RUNNINGED 3
#define FINISHED 4

#include <vector>
#include "flow.h"

using namespace std;

class Coflow {
public:
    Coflow(int coflowID, double startTime):coflowID(coflowID),startTime(startTime){
        coflowState = UNREGISTER;
        endflowNum = 0;
    }

    void addFlow(int flowID, int mapperID, int reducerID, double flowSizeMB);

    bool flowEnd(int flowID, int endtime);

    const int getCoflowID() const;

    const double getStartTime() const;

    void toString();

    int getCoflowState() const;
    void setCoflowState(int coflowState);

// TODO
    vector<Flow*> flowCollection;
private:
    const int coflowID;
    const double startTime;
    int coflowState;

    int endflowNum;
    int endtime;
    int mapper_num;
    int reducer_num;

};


#endif //MASTER_COFLOW_H
