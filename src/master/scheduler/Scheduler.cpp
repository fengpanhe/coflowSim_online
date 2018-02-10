//
// Created by he on 1/23/18.
//

#include <sstream>
#include <cstring>
#include "Scheduler.h"

void Scheduler::run() {
    startTime = time(0);



    bool flag = false;
    while(true){
        int delay = 10000000000;
        while(--delay > 0);
        if(machines->getPhysicsMachineNum() <= 0){
            continue;
        }
        printf("de\n");
        for (int i = 0; i < mUnregisterCoflows->size(); ++i) {
            Coflow* co = mUnregisterCoflows->at(i);
            for(auto &it:co->flowCollection){
                delay = 1000000000;
                while(--delay > 0);
                stringstream ss;
                ss << "coflowID:";
                ss << co->getCoflowID();
                ss << "||flow:";
                ss << it->getMapperID();
                ss << "->";
                ss << it->getReducerID();
                ss << "||";
                ss << it->getFlowSizeMB();
                char ch[100];
                ss >> ch;
                machines->sendTask(it->getMapperID(), ch, strlen(ch));
            }
        }
        if(flag) break;
    }
}

void Scheduler::setMachines(MachineManager *machines) {
    Scheduler::machines = machines;
}

bool Scheduler::setUnregisterCoflows(vector<Coflow *> *coflows) {
    mUnregisterCoflows = coflows;
    return true;
}
void Scheduler::registerCoflow()
{
    int nowTime = time(0);
    int t = nowTime - startTime;
    while (true){
        if(mUnregisterCoflows->front()->getStartTime() <= t){
            registedCoflows->push_back(mUnregisterCoflows->at(0));
            mUnregisterCoflows->erase(mUnregisterCoflows->begin());
        }
        else{
            break;
        }
    }
}
