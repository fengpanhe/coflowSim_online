//
// Created by he on 5/10/18.
//

#include "WebServer.h"
#include "../lib/epollFunctions.h"
const int BACKLOG = 5;

void WebServer::show_error(int connfd, const char *info) {
  printf("%s", info);
  send(connfd, info, strlen(info), 0);
  close(connfd);
}

WebServer::WebServer(const char *root_path, int max_thread_num) {
  this->root_path_ = new char[strlen(root_path)];
  strcpy(this->root_path_, root_path);
  http_conn::setDocRootPath(this->root_path_);

  this->max_thread_num_ = max_thread_num;

  try {
    this->pool_ = new ThreadPool(this->max_thread_num_);
  } catch (...) {
    return;
  }

  this->epollfd_ = epoll_create(5);
  assert(epollfd_ != -1);

  this->users_ = new http_conn[MAX_FD];
}
WebServer::~WebServer() {
  if (this->root_path_ != nullptr) {
    delete this->root_path_;
  }
  close(this->epollfd_);
  close(this->listenfd_);
  delete[] users_;
  delete pool_;
}
bool WebServer::addHandler(const char *match_url,
                           RequestHandler *request_handler) {
  return http_conn::addHandler(match_url, request_handler);
}
void WebServer::setListen(const char *ip, int listen_port) {

  this->listenfd_ = socket(PF_INET, SOCK_STREAM, 0);
  assert(this->listenfd_ >= 0);
  struct linger tmp = {1, 0};
  setsockopt(this->listenfd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
  int ret = 0;
  struct sockaddr_in address {};
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &address.sin_addr);
  address.sin_port = htons(static_cast<uint16_t>(listen_port));

  ret = bind(this->listenfd_, (struct sockaddr *)&address, sizeof(address));
  assert(ret >= 0);

  ret = listen(this->listenfd_, BACKLOG);
  assert(ret >= 0);
}
void WebServer::start() {
  addfd(this->epollfd_, this->listenfd_, false);
  http_conn::m_epollfd = this->epollfd_;

  int task_num = 0;
  while (true) {
    int number =
        epoll_wait(this->epollfd_, this->events_, MAX_EVENT_NUMBER, -1);
    if ((number < 0) && (errno != EINTR)) {
      printf("epoll failure\n");
      break;
    }

    for (int i = 0; i < number; i++) {
      int sockfd = this->events_[i].data.fd;
      if (sockfd == this->listenfd_) {
        printf("listenfd\n");
        struct sockaddr_in client_address {};
        socklen_t client_addrlength = sizeof(client_address);
        int connfd = accept(this->listenfd_, (struct sockaddr *)&client_address,
                            &client_addrlength);
        if (connfd < 0) {
          printf("errno is: %d\n", errno);
          continue;
        }
        if (http_conn::m_user_count >= MAX_FD) {
          show_error(connfd, "Internal server busy");
          continue;
        }
        printf("sockfd:%d\n", connfd);
        this->users_[connfd].init(connfd, client_address);
      } else if (this->events_[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        this->users_[sockfd].close_conn();
      } else if (this->events_[i].events & EPOLLIN) {
        if (this->users_[sockfd].read()) {
          this->pool_->append(this->users_ + sockfd);
          // printf("task_num: %d\n", ++task_num);
        } else {
          this->users_[sockfd].close_conn();
        }
      } else if (this->events_[i].events & EPOLLOUT) {
        if (!this->users_[sockfd].write()) {
          this->users_[sockfd].close_conn();
        }
      } else {
      }
    }
  }
}
