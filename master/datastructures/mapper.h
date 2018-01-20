//
// Created by he on 1/20/18.
//

#ifndef MASTER_MAPPER_H
#define MASTER_MAPPER_H

#include "machine.h"

class Mapper {
public:
    Mapper(Machine * machine);
    Machine *getMachine() const;
    void setMachine(Machine *machine);

private:
    Machine * machine;

};


#endif //MASTER_MAPPER_H
