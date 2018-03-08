//
// Created by he on 1/20/18.
//

#include <iostream>
#include "coflow.h"


void Coflow::addFlow(int flowID, int mapperID, int reducerID, double flowSizeMB) {
    this->flowCollection.push_back(new Flow(flowID, mapperID, reducerID, flowSizeMB));
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
int Coflow::getCoflowState() const
{
    return coflowState;
}
void Coflow::setCoflowState(int coflowState)
{
    Coflow::coflowState = coflowState;
}
bool Coflow::flowEnd(int flowID, int endtime)
{
    for(auto & it:flowCollection){
        if(it->getFlowID() == flowID){
            it->setFlowState(FLOWEND);
            it->setEndTime(endtime);
            endflowNum++;
            break;
        }
    }
    if(endflowNum >= flowCollection.size()){
        this->endtime = endtime;
        return true;
    }
    return false;
}
