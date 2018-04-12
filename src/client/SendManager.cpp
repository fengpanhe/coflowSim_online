//
// Created by he on 4/12/18.
//

#include <sstream>
#include "SendManager.h"

SendManager::SendManager(int task_number) {
  this->max_task_number = task_number;
}

bool SendManager::appendTask(char *ins) {
  SendTask send_task;
  stringstream ss;
  ss << ins;
  ss >> send_task.coflow_id;
  ss >> send_task.flow_id;
  ss >> send_task.destination_ip;
  ss >> send_task.destination_port;
  ss >> send_task.file_name;
  ss >> send_task.flow_size_MB;
  ss >> send_task.speed_Mbs;
  ss.clear();
  queue_locker.lock();
  if(send_task_queue.size() > max_task_number){
    printf("warning: send_task_queue_size(%d) > max_task_number(%d)",
           static_cast<int>(send_task_queue.size()), max_task_number);
    queue_locker.unlock();
    return false;
  }
  this->send_task_queue.push_back(send_task);
  queue_locker.unlock();
  queue_sem.post();
  return true;
}
