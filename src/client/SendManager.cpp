//
// Created by he on 4/12/18.
//

#include <sstream>
#include <asm/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <random>
#include "SendManager.h"
#include "Sender.h"

SendManager::SendManager(int task_number = 65535,
                         int min_port = 1001,
                         int max_port = 65535,
                         TrafficControlManager *tc_manager,
                         ThreadPool<ThreadClass> *pool) {
  this->max_task_number = task_number;
  this->min_port = min_port;
  this->max_port = max_port;
  run_stop = false;
}

void SendManager::run() {
  struct SendTask *send_task;
  struct sockaddr_in destination_addr{}, source_addr{};
  int connSockfd, local_port;;
  default_random_engine e;
  uniform_int_distribution<int> rand_num(this->min_port, this->max_port);
  memset(&destination_addr, 0, sizeof(struct sockaddr_in));
  memset(&source_addr, 0, sizeof(struct sockaddr_in));
  source_addr.sin_family = AF_INET;
  source_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  destination_addr.sin_family = AF_INET;
  int error = 0;
  int reuse = 1;

  while (!run_stop) {
    wait_queue_sem.wait();
    wait_queue_locker.lock();
    if (send_task_wait_queue.empty()) {
      wait_queue_locker.unlock();
      continue;
    }
    send_task = send_task_wait_queue.front();
    send_task_wait_queue.pop_front();
    wait_queue_locker.unlock();

    // 申请socketfd
    if ((connSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      printf("Failed to create socket\n");
    }
    // 为Socket绑定一个可用的端口
    local_port = rand_num(e);
    source_addr.sin_port = htons(static_cast<uint16_t>(local_port));
    while (bind(connSockfd, (struct sockaddr *) &source_addr, sizeof(struct sockaddr))==-1) {
      local_port = rand_num(e);
      source_addr.sin_port = htons(static_cast<uint16_t>(local_port));
    }
    // 建立连接
    destination_addr.sin_addr.s_addr = inet_addr(send_task->destination_ip);
    destination_addr.sin_port = htons(static_cast<uint16_t>(send_task->destination_port));
    if (connect(connSockfd, (struct sockaddr *) &destination_addr, sizeof(destination_addr)) <
        0) {
      printf("Failed to connect with server\n");
    }
    socklen_t len = sizeof(error);
    getsockopt(connSockfd, SOL_SOCKET, SO_ERROR, &error, &len);
    setsockopt(connSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 设定端口的速度
    this->tc_manager->setTcpPortBandwidth(local_port, send_task->speed_Mbs);
    // 建立一个发送任务
    send_task->sender = new Sender(send_task, connSockfd);
    pool->append(send_task->sender);

    // 将发送任务移动到send_task_running_queue
    send_task->send_state = SEND_RUNNING;
    send_task_running_queue.push_back(send_task);

    // 循环扫描 send_task_running_queue，处理标记完成的task
    for (auto &it : this->send_task_running_queue){
      if (it->send_state == SEND_END){
        // TODO 将it指向的task剔除，回传数据给master。
      }
    }

  }

}

bool SendManager::appendTask(char *ins) {
  auto *send_task = (struct SendTask *) malloc(sizeof(struct SendTask));
  stringstream ss;
  ss << ins;
  ss >> send_task->coflow_id;
  ss >> send_task->flow_id;
  ss >> send_task->destination_ip;
  ss >> send_task->destination_port;
  ss >> send_task->file_name;
  ss >> send_task->flow_size_MB;
  ss >> send_task->speed_Mbs;
  ss.clear();
  send_task->send_state = SEND_WAIT;
  wait_queue_locker.lock();
  if (send_task_wait_queue.size() > max_task_number) {
    printf("warning: send_task_queue_size(%d) > max_task_number(%d)",
           static_cast<int>(send_task_wait_queue.size()), max_task_number);
    wait_queue_locker.unlock();
    return false;
  }
  this->send_task_wait_queue.push_back(send_task);
  wait_queue_locker.unlock();
  wait_queue_sem.post();
  return true;
}

bool SendManager::createSendTask(struct SendTask *send_task) {

  return false;
}

