//
// Created by he on 1/20/18.
//

#include "coflowCollection.h"

CoflowCollection::CoflowCollection() {}

const vector<Coflow *> &CoflowCollection::getCoflows() const { return coflows; }

bool CoflowCollection::addCoflow(Coflow *coflow) {
  this->coflows.push_back(coflow);
  return true;
}

int CoflowCollection::getCoflowNum() const { return coflowNum; }

void CoflowCollection::setCoflowNum(int coflowNum) {
  CoflowCollection::coflowNum = coflowNum;
}

Coflow *CoflowCollection::getCoflowByindex(int index) {
  return this->coflows[index];
}
