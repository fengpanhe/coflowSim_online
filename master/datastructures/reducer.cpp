//
// Created by he on 1/20/18.
//

#include "reducer.h"

Reducer::Reducer(Machine *machine) {
    setMachine(machine);
}

Machine *Reducer::getMachine() const {
    return machine;
}

void Reducer::setMachine(Machine *machine) {
    Reducer::machine = machine;
}
