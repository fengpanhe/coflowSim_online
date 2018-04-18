#ifndef COFLOWSIM_LISTENER_H
#define COFLOWSIM_LISTENER_H

#include <arpa/inet.h>
#include <lib/epollFunctions.h>
#include <lib/threadclass.h>
#include <netinet/in.h>
#include <cstdio>
#include <strings.h>
#include <lib/threadpool.h>
#include "receFile.h"

#define MAX_EVENT_NUMBER 10000
#define BACKLOG 65535
#define RECV_FILE_NUM 65536

class RecvManager : public ThreadClass {
public:
  RecvManager(ThreadPool<ThreadClass> *pool, int listen_sockfd);
//  ~RecvManager(){}
  void run() override;
private:
  bool closeListen(int sockfd);
  int createListen(char * listen_ip, int port);

  int epollfd;
  epoll_event events[MAX_EVENT_NUMBER];

  ReceFile receFiles[RECV_FILE_NUM];
  ThreadPool<ThreadClass> *pool;
  int listen_sockfd;
};
#endif // !LISTENER_H