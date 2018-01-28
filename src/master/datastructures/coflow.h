//
// Created by he on 1/20/18.
//

#ifndef MASTER_COFLOW_H
#define MASTER_COFLOW_H

#include <vector>
#include "flow.h"

using namespace std;

class Coflow {
public:
    Coflow(int coflowID, double startTime):coflowID(coflowID),startTime(startTime){}

    void addFlow(int mapperID, int reducerID, double flowSizeMB);

    const int getCoflowID() const;

    const double getStartTime() const;

    void toString();
// TODO
    vector<Flow*> flowCollection;
private:
    const int coflowID;
    const double startTime;
    int mapper_num;
    int reducer_num;

};


#endif //MASTER_COFLOW_H