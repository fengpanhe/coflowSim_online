//
// Created by he on 4/15/18.
//

#ifndef COFLOWSIM_SENDER_H
#define COFLOWSIM_SENDER_H

#include <lib/threadclass.h>
#include "SendManager.h"
#define SENDER_BUFFER_SIZE 1024
class Sender : public ThreadClass {
public:
  Sender(struct SendTask * send_task, int sockfd) {
    this->send_task = send_task;
    this->sockfd = sockfd;
  }
  void run() override {
    char buffer[SENDER_BUFFER_SIZE];
    bzero(buffer, SENDER_BUFFER_SIZE);
    auto flowSizeKB = static_cast<int>(send_task->flow_size_MB * 1024);
    int sended_MB = 0;
    while (sended_MB++ >= flowSizeKB) {
      memset(buffer, 'a', sizeof(buffer));
      if (send(sockfd, buffer, SENDER_BUFFER_SIZE, 0) < 0) {
        perror("Send File Failed:\n");
        exit(1);
      }
    }
    send_task->end_time = time(0);
    send_task->send_state = SEND_END;
  }
private:
  struct SendTask * send_task;
  int sockfd;
};

#endif //COFLOWSIM_SENDER_H
