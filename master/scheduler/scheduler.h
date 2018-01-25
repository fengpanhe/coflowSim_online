//
// Created by he on 1/23/18.
//

#ifndef MASTER_SCHEDULER_H
#define MASTER_SCHEDULER_H


#include "../datastructures/coflow.h"
#include "lib/thread.h"
#include "../datastructures/machine.h"
#include "machineManager.h"

class scheduler : public thread {
public:
    void run() override;

    bool onCoflowRegister(Coflow* c);

    bool onCoflowUnregister(Coflow* c);

    bool setUnregisterCoflows(vector<Coflow*>* coflows);

private:
    vector<Coflow*>* mUnregisterCoflows;
    vector<Coflow*>* mRunningCoflows;
    vector<Coflow*>* mFinishedCoflows;
    vector<Coflow*>* mNotAdmittedCoflows;

//    TODO
    machineManager* machines;
public:
    void setMachines(machineManager *machines);
};


#endif //MASTER_SCHEDULER_H
