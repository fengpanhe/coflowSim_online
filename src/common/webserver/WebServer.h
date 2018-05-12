//
// Created by he on 5/10/18.
//

#ifndef WEB_SERVER_CPP_WEBSERVER_H
#define WEB_SERVER_CPP_WEBSERVER_H

#include "http_conn.h"
class WebServer {
public:
  static const int MAX_EVENT_NUMBER = 10000;
  static const int MAX_FD = 65536;
  WebServer(const char *root_path = "./", int max_thread_num = 8);
  ~WebServer();
  bool addHandler(const char *match_url, RequestHandler *request_handler);
  void setListen(const char *ip, int listen_port = 3000);
  void start();

private:
  int epollfd_;
  epoll_event events_[MAX_EVENT_NUMBER];

  ThreadPool *pool_;

  http_conn *users_;
  int listenfd_;

  char *root_path_;
  int max_thread_num_;

  void show_error(int connfd, const char *info);
};

#endif // WEB_SERVER_CPP_WEBSERVER_H
