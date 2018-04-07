//
// Created by he on 1/20/18.
//

#include "coflow.h"
#include <iostream>

void Coflow::addFlow(int flowID, int mapperID, int reducerID,
                     double flowSizeMB) {
  this->flowCollection[flowID] =
      new Flow(flowID, mapperID, reducerID, flowSizeMB);
}

const int Coflow::getCoflowID() const { return coflowID; }

const double Coflow::getStartTime() const { return startTime; }

int Coflow::getFlowsNum() const { return this->flowCollection.size(); }
void Coflow::toString() {
  cout << "coflowID:" << this->getCoflowID() << endl;
  cout << "startTime:" << this->getStartTime() << endl;
  //   for (auto &it : this->flowCollection) {
  //     it->second;
  //   }
}
int Coflow::getCoflowState() const { return coflowState; }

void Coflow::setCoflowState(int coflowState) {
  Coflow::coflowState = coflowState;
}

bool Coflow::flowEnd(int flowID, int endtime) {
  FLOWS_MAP_TYPE_IT it = this->flowCollection.find(flowID);
  if (it == this->flowCollection.end()) {
    return false;
  }

  it->second->setFlowState(FLOWEND);
  it->second->setEndTime(endtime);
  endflowNum++;
  //   for (auto &it : flowCollection) {
  //     if (it->getFlowID() == flowID) {
  //       it->setFlowState(FLOWEND);
  //       it->setEndTime(endtime);
  //       endflowNum++;
  //       break;
  //     }
  //   }
  if (endflowNum >= flowCollection.size()) {
    this->endtime = endtime;
    return true;
  }
  return false;
}

FLOWS_MAP_TYPE_IT Coflow::flowsBegin() { return this->flowCollection.begin(); }
FLOWS_MAP_TYPE_IT Coflow::flowsEnd() { return this->flowCollection.end(); }
