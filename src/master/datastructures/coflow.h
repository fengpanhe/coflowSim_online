//
// Created by he on 1/20/18.
//

#ifndef MASTER_COFLOW_H
#define MASTER_COFLOW_H

#include "flow.h"
#include "rapidjson/stringbuffer.h"
#include <map>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
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
  Coflow(int coflowID, long registerTime)
      : coflowID(coflowID), registerTime(registerTime) {
    coflowState = UNREGISTER;
    end_flow_num_ = 0;
    sended_size_ = 0;
    this->start_time_ = -1;
    this->end_time_ = -1;
  }
  ~Coflow();

  void addFlow(int flowID, int mapperID, int reducerID, double flowSizeMB);

  bool flowEnd(int flowID, int endtime);

  int getFlowsNum() const;

  const int getCoflowID() const;

  const double getRegisterTime() const;
  long getStartTime() const;
  void setStartTime(long start_time);

  void calcCoflowLength();
  void calcCoflowWidth();
  void calcCoflowSize();
  void calcCoflowSkew();

  double getCoflowLength() const;
  int getCoflowWidth() const;
  double getCoflowSizeMbit() const;
  double getCoflowSkew() const;
  long getEndTime() const;
  void setEndTime(long end_time_);
  int getEndflowNum() const;
  void setEndflowNum(int endflowNum);
  double getSendedSize() const;

  string getCoflowJson();

  void toString();

  int getCoflowState() const;

  void setCoflowState(int coflowState);

  FLOWS_MAP_TYPE_IT flowsBegin();
  FLOWS_MAP_TYPE_IT flowsEnd();

private:
  const int coflowID;
  const long registerTime;
  long start_time_;
  long end_time_;
  int coflowState;

  double coflow_length_;
  int coflow_width_;
  double coflow_size_Mbit_;
  double coflow_skew_;

  // vector<Flow *> flowCollection;
  FLOWS_MAP_TYPE flowCollection;
  int end_flow_num_;
  double sended_size_;

//  int endtime;
  int mapper_num;
  int reducer_num;
};

#endif // MASTER_COFLOW_H
