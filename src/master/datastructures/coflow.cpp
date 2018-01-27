//
// Created by he on 1/20/18.
//

#include <iostream>
#include "coflow.h"


void Coflow::addFlow(int mapperID, int reducerID, double flowSizeMB) {
    this->flowCollection.push_back(new Flow(mapperID, reducerID, flowSizeMB));
}

const int Coflow::getCoflowID() const {
    return coflowID;
}

const double Coflow::getStartTime() const {
    return startTime;
}

void Coflow::toString() {
    cout << "coflowID:" << this->getCoflowID() << endl;
    cout << "startTime:" << this->getStartTime() << endl;
    for(auto & it:this->flowCollection){
        it->toString();
    }
}
