//
// Created by he on 4/15/18.
//

#ifndef COFLOWSIM_SENDER_H
#define COFLOWSIM_SENDER_H

#include <lib/threadclass.h>
class Sender : public ThreadClass {
public:
  Sender(struct SendTask * send_task, int sockfd);
  void run();

};

#endif //COFLOWSIM_SENDER_H
