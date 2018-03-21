//
// Created by he on 3/12/18.
//

#ifndef COFLOWSIM_RECVLISTEN_H
#define COFLOWSIM_RECVLISTEN_H

#include "receFile.h"
#include <lib/threadclass.h>
#include <lib/threadpool.h>
#include <netinet/in.h>
#include <sys/socket.h>
class RecvListen : public ThreadClass {
public:
  RecvListen(int lSockfd, ThreadPool<ThreadClass> *pool) {
    listenSockfd = lSockfd;
    receFiles = new ReceFile[65536];
    this->pool = pool;
  }
  void run() {
    while (true) {
      struct sockaddr_in client_address {};
      socklen_t client_addrlength = sizeof(client_address);
      int connfd = accept(listenSockfd, (struct sockaddr *)&client_address,
                          &client_addrlength);
      if (connfd < 0) {
        continue;
      }
      receFiles[connfd].initSocket(connfd, client_address);
      pool->append(receFiles + connfd);
    }
  }

private:
  int listenSockfd;
  ReceFile *receFiles;
  ThreadPool<ThreadClass> *pool;
};
#endif // COFLOWSIM_RECVLISTEN_H
