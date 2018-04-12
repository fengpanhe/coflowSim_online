#ifndef LISTENER_H
#define LISTENER_H

#include <arpa/inet.h>
#include <lib/epollFunctions.h>
#include <lib/threadclass.h>
#include <netinet/in.h>
#include <cstdio>
#include <strings.h>

#define MAX_EVENT_NUMBER 10000
#define BACKLOG 65535
char serverIP[64] = "127.0.0.1";
class Listener : public ThreadClass {
public:
  Listener();
  ~Listener();
  void run();

  bool createListen(int port);
  bool closeListten(int sockfd);

private:
  int epollfd;
  epoll_event events[MAX_EVENT_NUMBER];
};
#endif // !LISTENER_H