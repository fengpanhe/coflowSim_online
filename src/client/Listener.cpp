#include <cassert>
#include <cerrno>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <iostream>
#include "Listener.h"

static std::shared_ptr<spdlog::logger> Listener_logger_console = NULL;

static std::shared_ptr<spdlog::logger> Listener_file_logger = NULL;

Listener::Listener(ThreadPool<ThreadClass> *pool, int listen_port) {
  epollfd = epoll_create(MAX_EVENT_NUMBER);
  assert(epollfd != -1);
  this->pool = pool;
  this->listen_port = listen_port;
//  this->run_stop = false;

  if (Listener_logger_console == NULL) {
    Listener_logger_console =
        spdlog::stdout_color_mt("Listener_logger");
  }
  if (Listener_file_logger == NULL) {
    try {
      spdlog::set_async_mode(8192);
      Listener_file_logger = spdlog::rotating_logger_mt(
          "socketManage_file_logger", "Listener_file_logger.log",
          1024 * 1024 * 5, 3);
      spdlog::drop_all();
    } catch (const spdlog::spdlog_ex &ex) {
      std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
  }
}

void Listener::run() {
  int listenSockfd;
  listenSockfd = this->createListen(this->listen_port);
  addfd(this->epollfd, listenSockfd, false);
  while (!this->run_stop) {
    int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
    if ((number < 0) && (errno != EINTR)) {
      Listener_logger_console->error("epoll failure\n");
      break;
    }
    for (int i = 0; i < number; i++) {
      int listenfd = events[i].data.fd;

      struct sockaddr_in client_address {};
      socklen_t client_addrlength = sizeof(client_address);
      int connfd = accept(listenfd, (struct sockaddr *)&client_address,
                          &client_addrlength);
      if (connfd < 0) {
        Listener_logger_console->error("errno is: {}\n", errno);
        continue;
      }
      receFiles[connfd].initSocket(connfd, client_address);
      pool->append(receFiles + connfd);
//      this->closeListen(listenfd);
    }
  }
}

int Listener::createListen(int port) {
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
  return listenSockfd;
}
bool Listener::closeListen(int sockfd) {
  if (sockfd != -1) {
    removefd(this->epollfd, sockfd);
  }
  return true;
}
