//
// Created by he on 1/23/18.
//

#include <sstream>
#include "scheduler.h"

void scheduler::run() {
    bool flag = false;
    while(true){
        int delay = 100000000;
        while(--delay > 0);
        printf("de\n");
//        m_logicMap[1]->setSendMsg(ch, 10);
        for (auto &it:machines->m_logicMap){
            delay = 100000000;
            while(--delay > 0);
            stringstream ss;
            ss << it.first;
            char ch[10];
            ss >> ch;
            printf("",ch);
            it.second->setSendMsg(ch, 10);
            flag = true;
        }
        if(flag) break;
    }
}

void scheduler::setMachines(machineManager *machines) {
    scheduler::machines = machines;
}
