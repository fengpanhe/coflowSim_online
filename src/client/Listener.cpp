#include <cassert>
#include "Listener.h"

Listener::Listener() {
  epollfd = epoll_create(MAX_EVENT_NUMBER);
  assert(epollfd != -1);
}

Listener::~Listener() {}

bool Listener::createListen(int port) {
  int listenSockfd;
  struct sockaddr_in listenAddr {};
  // 初始化监听socket
  if ((listenSockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Creating listenSocket failed");
    return -1;
  }
  struct linger tmp = {1, 0};
  setsockopt(listenSockfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

  bzero(&listenAddr, sizeof(listenAddr));
  listenAddr.sin_family = AF_INET;
  inet_pton(AF_INET, serverIP, &listenAddr.sin_addr);
  listenAddr.sin_port = htons(static_cast<uint16_t>(port));
  if (bind(listenSockfd, (struct sockaddr *)&listenAddr,
           sizeof(struct sockaddr)) == -1) {
    perror("Bind error\n");
    return -1;
  }
  if (listen(listenSockfd, BACKLOG) == -1) {
    perror("listen() error\n");
    return -1;
  }
//  console->info("Successfully initialized listenSockfd and address!");
}