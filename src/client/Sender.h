//
// Created by he on 4/15/18.
//

#ifndef COFLOWSIM_SENDER_H
#define COFLOWSIM_SENDER_H

#include <lib/threadclass.h>
#include "SendManager.h"

class Sender : public ThreadClass {
public:
  Sender(struct SendTask * send_task, int sockfd) {
    this->send_task = send_task;
    this->sockfd = sockfd;
  }
  void run() override {

  }
private:
  struct SendTask * send_task;
  int sockfd;
};

#endif //COFLOWSIM_SENDER_H
