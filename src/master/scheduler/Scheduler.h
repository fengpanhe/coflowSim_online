//
// Created by he on 1/23/18.
//

#ifndef MASTER_SCHEDULER_H
#define MASTER_SCHEDULER_H

#include <queue>
#include "../datastructures/coflow.h"
#include "../datastructures/machine.h"
#include "lib/threadclass.h"
#include "MachineManager.h"

class Scheduler : public ThreadClass {
public:
    Scheduler(){
        registerIndex = 0;
    }

    void run() override;

    bool onCoflowRegister(Coflow* c);

    bool onCoflowUnregister(Coflow* c);

    bool setCoflows(vector<Coflow*>* coflows);

    // 模拟coflow的注册过程
    void registerCoflow();
private:
    vector<Coflow*>* sCoflows;
    int registerIndex;

    vector<Coflow*>* registedCoflows;
    vector<Coflow*>* mRunningCoflows;
    vector<Coflow*>* mFinishedCoflows;
    vector<Coflow*>* mNotAdmittedCoflows;

    //    TODO
    MachineManager* machines;

    int startTime;

public:
    void setMachines(MachineManager* machines);
};

#endif // MASTER_SCHEDULER_H
