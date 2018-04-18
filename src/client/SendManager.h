//
// Created by he on 4/12/18.
//

#ifndef COFLOWSIM_SENDMANAGER_H
#define COFLOWSIM_SENDMANAGER_H

#include <list>
#include <lib/locker.h>
#include <lib/threadclass.h>
#include <lib/threadpool.h>
#include <socket/socketManage.h>
#include "TrafficControlManager.h"
using namespace std;

#define FILE_NAME_MAX_SIZE 32
#define SEND_WAIT 0
#define SEND_RUNNING 1
#define SEND_END 2
#define TASK_END 3

class Sender;

struct SendTask {
  char destination_ip[64] = "";
  int destination_port = 0;
  double flow_size_MB = 0;
  double speed_Mbs = 0;
  char file_name[FILE_NAME_MAX_SIZE] = "";
  int coflow_id = -1;
  int flow_id = -1;

  int send_state{};
  long end_time{};
  Sender *sender{};
};

class SendManager : public ThreadClass {
public:
  SendManager(TrafficControlManager *tc_manager,
              ThreadPool<ThreadClass> *pool,
              SocketManage *masterSockManger,
              int task_number = 65536,
              int min_port = 1001,
              int max_port = 65535);
  bool appendTask(char *ins, int ins_len);
  void run() override;

private:

  int max_task_number;  // 最大任务数量

  list<struct SendTask *> send_task_wait_queue;   // 发送任务等待队列
  locker wait_queue_locker;
  sem wait_queue_sem;

  list<struct SendTask *> send_task_running_queue; // 发送任务正在进行队列
  locker running_queue_locker;

  bool run_stop;  // 运行停止

  int min_port;   // 最小端口号
  int max_port;   // 最大端口号

  TrafficControlManager *tc_manager;
  ThreadPool<ThreadClass> *pool;
  SocketManage *masterSockManger;

};

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
    while (sended_MB++ < flowSizeKB) {
      memset(buffer, 'a', sizeof(buffer));
      if (send(sockfd, buffer, SENDER_BUFFER_SIZE, 0) < 0) {
        perror("Send File Failed:\n");
        exit(1);
      }
    }
    send_task->end_time = time(0);
    send_task->send_state = SEND_END;
    close(sockfd);
  }
private:
  struct SendTask * send_task;
  int sockfd;
};

#endif //COFLOWSIM_SENDMANAGER_H
