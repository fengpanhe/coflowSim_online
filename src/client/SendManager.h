//
// Created by he on 4/12/18.
//

#ifndef COFLOWSIM_SENDMANAGER_H
#define COFLOWSIM_SENDMANAGER_H

#include <list>
#include <lib/locker.h>
using namespace std;

#define FILE_NAME_MAX_SIZE 32
struct SendTask{
  char destination_ip[64] = "";
  int destination_port = 0;
  double flow_size_MB = 0;
  double speed_Mbs = 0;
  char file_name[FILE_NAME_MAX_SIZE] = "";
  int coflow_id = -1;
  int flow_id = -1;
};

class SendManager {
public:
  SendManager(int task_number = 65536);
  bool appendTask(char *ins);

private:
  int max_task_number;
  list<SendTask> send_task_queue;
  locker queue_locker;
  sem queue_sem;

};

#endif //COFLOWSIM_SENDMANAGER_H
