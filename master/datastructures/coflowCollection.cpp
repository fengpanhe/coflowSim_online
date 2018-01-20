//
// Created by he on 1/20/18.
//

#include "coflowCollection.h"

CoflowCollection::CoflowCollection() {

}

const vector<Coflow *> &CoflowCollection::getCoflows() const {
    return coflows;
}

bool CoflowCollection::addCoflow(Coflow* flow) {
    this->coflows.push_back(flow);
    return true;
}
