//
// Created by he on 1/20/18.
//

#include "coflow.h"
#include <iostream>

Coflow::~Coflow() {
  for (auto &it : flowCollection) {
    if (it.second != nullptr) {
      delete it.second;
    }
  }
}

void Coflow::addFlow(int flowID, int mapperID, int reducerID,
                     double flowSizeMB) {
  this->flowCollection[flowID] =
      new Flow(flowID, mapperID, reducerID, flowSizeMB);
}

const int Coflow::getCoflowID() const { return coflowID; }

const double Coflow::getRegisterTime() const { return registerTime; }

int Coflow::getFlowsNum() const { return this->flowCollection.size(); }
void Coflow::toString() {
  cout << "coflowID:" << this->getCoflowID() << endl;
  cout << "registerTime:" << this->getRegisterTime() << endl;
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
  end_flow_num_++;
  this->sended_size_ += it->second->getFlowSizeMB();
  //   for (auto &it : flowCollection) {
  //     if (it->getFlowID() == flowID) {
  //       it->setFlowState(FLOWEND);
  //       it->setEndTime(endtime);
  //       end_flow_num_++;
  //       break;
  //     }
  //   }
  if (end_flow_num_ >= flowCollection.size()) {
    this->end_time_ = endtime;
    return true;
  }
  return false;
}

FLOWS_MAP_TYPE_IT Coflow::flowsBegin() { return this->flowCollection.begin(); }
FLOWS_MAP_TYPE_IT Coflow::flowsEnd() { return this->flowCollection.end(); }
void Coflow::calcCoflowLength() {
  this->coflow_length_ = 0;
  for (auto &it : flowCollection) {
    if (it.second->getFlowSizeMB() > this->coflow_length_) {
      this->coflow_length_ = it.second->getFlowSizeMB();
    }
  }
}
void Coflow::calcCoflowWidth() {
  this->coflow_width_ = static_cast<int>(flowCollection.size());
}
void Coflow::calcCoflowSize() {
  this->coflow_size_Mbit_ = 0;
  for (auto &it : flowCollection) {
    this->coflow_size_Mbit_ += it.second->getFlowSizeMB();
  }
}
void Coflow::calcCoflowSkew() {
  double max_flow_size = flowCollection.begin()->second->getFlowSizeMB();
  double min_flow_size = flowCollection.begin()->second->getFlowSizeMB();
  for (auto &it : flowCollection) {
    if (it.second->getFlowSizeMB() > max_flow_size) {
      max_flow_size = it.second->getFlowSizeMB();
    }
    if (it.second->getFlowSizeMB() < min_flow_size) {
      min_flow_size = it.second->getFlowSizeMB();
    }
  }
  this->coflow_skew_ = (max_flow_size - min_flow_size) / min_flow_size;
}
double Coflow::getCoflowLength() const { return coflow_length_; }
int Coflow::getCoflowWidth() const { return coflow_width_; }
double Coflow::getCoflowSizeMbit() const { return coflow_size_Mbit_; }
double Coflow::getCoflowSkew() const { return coflow_skew_; }
string Coflow::getCoflowJson() {
  rapidjson::Document coflow_json;
  coflow_json.SetObject();
  rapidjson::Document::AllocatorType &allocator = coflow_json.GetAllocator();
  coflow_json.AddMember("CoflowID", rapidjson::Value(this->getCoflowID()),
                        allocator);
  coflow_json.AddMember("RegisterTime",
                        rapidjson::Value(this->getRegisterTime()), allocator);
  coflow_json.AddMember("StartTime", rapidjson::Value(this->getStartTime()),
                        allocator);
  coflow_json.AddMember("EndTime", rapidjson::Value(this->getEndTime()),
                        allocator);
  coflow_json.AddMember("CoflowLength",
                        rapidjson::Value(this->getCoflowLength()), allocator);
  coflow_json.AddMember("CoflowWidth", rapidjson::Value(this->getCoflowWidth()),
                        allocator);
  coflow_json.AddMember("CoflowSize",
                        rapidjson::Value(this->getCoflowSizeMbit()), allocator);
  coflow_json.AddMember("CoflowSkew", rapidjson::Value(this->getCoflowSkew()),
                        allocator);
  coflow_json.AddMember("endFlowNum", rapidjson::Value(this->getEndflowNum()),
                        allocator);
  coflow_json.AddMember("sendedSize", rapidjson::Value(this->getSendedSize()),
                        allocator);
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  coflow_json.Accept(writer);
  string str = buffer.GetString();
  return str;
}
long Coflow::getStartTime() const { return start_time_; }
void Coflow::setStartTime(long start_time) { this->start_time_ = start_time; }
long Coflow::getEndTime() const { return end_time_; }
void Coflow::setEndTime(long end_time_) { Coflow::end_time_ = end_time_; }
int Coflow::getEndflowNum() const { return end_flow_num_; }
void Coflow::setEndflowNum(int endflowNum) { Coflow::end_flow_num_ = endflowNum; }
double Coflow::getSendedSize() const { return sended_size_; }
