//
// Created by he on 5/14/18.
//

#ifndef COFLOWSIM_ONLINE_COFLOWJSONHANDLER_H
#define COFLOWSIM_ONLINE_COFLOWJSONHANDLER_H
#include "../datastructures/coflow.h"
#include "webserver/http_conn.h"
#include <string>
using namespace std;
class CoflowJsonHandler : public RequestHandler {
public:
  CoflowJsonHandler(vector<Coflow *> *coflows) {
    coflows_ = coflows;
    coflow_json_ = new char[coflows_->size() * 1024];
  }
  ~CoflowJsonHandler() {}
  bool get(http_conn *http_c) override {
    http_c->setFileRelativePath("/index.html");
  }
  bool post(http_conn *http_c) override {
    strcpy(coflow_json_, "{\"returnCode\":1,\"coflows\":[");
    bool flag = false;
    for (int i = 0; i < coflows_->size(); i++) {
      if (coflows_->at(i)->getCoflowState() != UNREGISTER) {
        if (flag)
          strcat(coflow_json_, ",");
        else
          flag = true;
        strcat(coflow_json_, coflows_->at(i)->getCoflowJson().c_str());

      } else {
        break;
      }
    }
    strcat(coflow_json_, "]}");
    printf("coflow_json_size: %d\n", strlen(coflow_json_));
    http_c->setResponseContent(coflow_json_);
  }

private:
  vector<Coflow *> *coflows_;
  char *coflow_json_;
};

#endif // COFLOWSIM_ONLINE_COFLOWJSONHANDLER_H
