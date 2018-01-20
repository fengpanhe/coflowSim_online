//
// Created by he on 1/20/18.
//

#ifndef MASTER_REDUCER_H
#define MASTER_REDUCER_H


#include "machine.h"

class Reducer {

public:
    Reducer(Machine* machine);

    Machine *getMachine() const;

    void setMachine(Machine *machine);

private:
    Machine* machine;
};


#endif //MASTER_REDUCER_H
