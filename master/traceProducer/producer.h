//
// Created by he on 1/21/18.
//

#ifndef MASTER_PRODUCER_H
#define MASTER_PRODUCER_H


#include "../datastructures/coflowCollection.h"

class Producer {
public:
    virtual CoflowCollection* prepareCoflows() = 0;
    virtual vector<Machine*>* prepareMachines() = 0;
    virtual int getCoflowNum() = 0;
};


#endif //MASTER_PRODUCER_H
