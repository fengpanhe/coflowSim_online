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
    Coflow(int coflowID, double statrtTime):coflowID(coflowID),startTime(startTime){}
    void addFlow(Mapper* mapper, Reducer* reducer, double flowSizeMB);

private:
    const int coflowID;
    const double startTime;
    int mapper_num;
    int reducer_num;
    vector<Flow*> flowCollection;
};


#endif //MASTER_COFLOW_H
