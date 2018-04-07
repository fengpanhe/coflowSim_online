//
// Created by he on 1/20/18.
//

#ifndef MASTER_COFLOW_H
#define MASTER_COFLOW_H

#include "flow.h"
#include <map>
#include <vector>
using namespace std;

#define UNREGISTER 0
#define REGISTED 1
#define RUNNING 2
#define RUNNINGED 3
#define FINISHED 4

typedef map<int, Flow *> FLOWS_MAP_TYPE;
typedef FLOWS_MAP_TYPE::iterator FLOWS_MAP_TYPE_IT;

class Coflow {
public:
  Coflow(int coflowID, double startTime)
      : coflowID(coflowID), startTime(startTime) {
    coflowState = UNREGISTER;
    endflowNum = 0;
  }

  void addFlow(int flowID, int mapperID, int reducerID, double flowSizeMB);

  bool flowEnd(int flowID, int endtime);

  int getFlowsNum() const;

  const int getCoflowID() const;

  const double getStartTime() const;

  void toString();

  int getCoflowState() const;

  void setCoflowState(int coflowState);

  FLOWS_MAP_TYPE_IT flowsBegin();
  FLOWS_MAP_TYPE_IT flowsEnd();

private:
  const int coflowID;
  const double startTime;
  int coflowState;

  // vector<Flow *> flowCollection;
  FLOWS_MAP_TYPE flowCollection;
  int endflowNum;
  int endtime;
  int mapper_num;
  int reducer_num;
};

#endif // MASTER_COFLOW_H
