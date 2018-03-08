//
// Created by he on 1/20/18.
//

#include "flow.h"
#include <iostream>
using namespace std;

void Flow::toString(){
    cout << "flow:" << this->mapperID << " -> " << this->reducerID << " : " << remainMB << "/" << this->flowSizeMB << " : " << this->currentMbs << "MBs" << endl;
}

const int Flow::getMapperID() const {
    return mapperID;
}

const int Flow::getReducerID() const {
    return reducerID;
}

const double Flow::getFlowSizeMB() const {
    return flowSizeMB;
}

double Flow::getRemainMB() const {
    return remainMB;
}

void Flow::setRemainMB(double remainMB) {
    Flow::remainMB = remainMB;
}

double Flow::getCurrentMbs() const {
    return currentMbs;
}

void Flow::setCurrentMbs(double currentMbs) {
    Flow::currentMbs = currentMbs;
}
const int Flow::getFlowID() const
{
    return flowID;
}
bool Flow::isFlowState() const
{
    return flowState;
}
void Flow::setFlowState(bool flowState)
{
    Flow::flowState = flowState;
}
int Flow::getEndTime() const
{
    return endTime;
}
void Flow::setEndTime(int endTime)
{
    Flow::endTime = endTime;
}
