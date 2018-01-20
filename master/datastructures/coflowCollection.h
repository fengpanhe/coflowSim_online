//
// Created by he on 1/20/18.
//

#ifndef MASTER_COFLOWCOLLECTION_H
#define MASTER_COFLOWCOLLECTION_H


#include "coflow.h"

class CoflowCollection {
public:
    CoflowCollection();

    const vector<Coflow *> &getCoflows() const;

    bool addCoflow(Coflow* flow);
private:
    vector<Coflow*> coflows;

};


#endif //MASTER_COFLOWCOLLECTION_H
