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

    bool addCoflow(Coflow* coflow);

    Coflow* getCoflowByindex(int index);

    int getCoflowNum() const;

    void setCoflowNum(int coflowNum);

private:
    vector<Coflow*> coflows;
    int coflowNum;

};


#endif //MASTER_COFLOWCOLLECTION_H
