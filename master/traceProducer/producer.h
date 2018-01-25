//
// Created by he on 1/21/18.
//

#ifndef MASTER_PRODUCER_H
#define MASTER_PRODUCER_H


#include "../datastructures/coflowCollection.h"
#include "../datastructures/machine.h"

class Producer {
public:
    virtual bool prepareCoflows(vector<Coflow*>* & coflows) = 0;
    virtual vector<Machine*>* prepareMachines() = 0;
    virtual int getCoflowNum() = 0;
};


#endif //MASTER_PRODUCER_H
