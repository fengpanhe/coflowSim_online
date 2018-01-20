//
// Created by he on 1/20/18.
//

#include "coflow.h"


void Coflow::addFlow(Mapper* mapper, Reducer* reducer, double flowSizeMB) {
    auto* flow = new Flow(mapper, reducer, flowSizeMB);
    this->flowCollection.push_back(flow);
}
