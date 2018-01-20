//
// Created by he on 1/20/18.
//

#include "mapper.h"

Mapper::Mapper(Machine *machine) {
    setMachine(machine);
}

Machine *Mapper::getMachine() const {
    return machine;
}

void Mapper::setMachine(Machine *machine) {
    Mapper::machine = machine;
}

