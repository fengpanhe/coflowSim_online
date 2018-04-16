#include "lib/epollFunctions.h"
#include "receFile.h"
#include "sendFile.h"
#include "socket/socketManage.h"
#include "spdlog/spdlog.h"
#include <arpa/inet.h>
#include <assert.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/epoll.h>
#include <zconf.h>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "recvListen.h"
#include "Listener.h"
using namespace rapidjson;
using namespace std;
#define CONN_PORT 4002

#define BACKLOG 65535
#define MAX_EVENT_NUMBER 10000

char serverIP[64] = "127.0.0.1";
int serverPort = 4003;
int threadNum = 8;
char *recvbuf = new char[4096];
int recvbuflen = 0;

bool parseConfig(char const *configFilePath);
int coflowSimClient();

namespace spd = spdlog;
int main(int argc, char const *argv[]) {
  auto console = spd::stdout_color_mt("console");
  parseConfig(argv[1]);
  coflowSimClient();
  return 0;
}

auto console = spdlog::stdout_color_mt("coflowSimClient");
auto coflowSimClient_logger =
    spdlog::basic_logger_mt("coflowSimClient_logger", "coflowSimClient.log");
int coflowSimClient() {
  // 创建线程池
  ThreadPool<ThreadClass> *pool = nullptr;
  try {
    pool = new ThreadPool<ThreadClass>(threadNum);
  } catch (...) {
    return 1;
  }

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
  listenAddr.sin_port = htons(static_cast<uint16_t>(serverPort));
  if (bind(listenSockfd, (struct sockaddr *)&listenAddr,
           sizeof(struct sockaddr)) == -1) {
    perror("Bind error\n");
    return -1;
  }
  if (listen(listenSockfd, BACKLOG) == -1) {
    perror("listen() error\n");
    return -1;
  }
  console->info("Successfully initialized listenSockfd and address!");

  // epoll创建
  epoll_event events[MAX_EVENT_NUMBER];
  int epollfd = epoll_create(MAX_EVENT_NUMBER);
  assert(epollfd != -1);

  SocketManage::sEpollfd = epollfd; // SocketManage类的静态变量赋值
  console->info("epoll_event is created!");

  // masterSockManger的创建，接收master的连接
  SocketManage masterSockManger;
  struct sockaddr_in master_address {};
  socklen_t master_addrlength = sizeof(master_address);

  console->info("Waiting for master...");
  int masterfd = accept(listenSockfd, (struct sockaddr *)&master_address,
                        &master_addrlength);
  masterSockManger.initSocket(masterfd, master_address);

  console->info("masterSockManger is ready!");

  vector<SendFile *> sendFiles;
  ReceFile *receFiles = new ReceFile[65536];

  //  addfd(epollfd, listenSockfd, false);
  RecvListen *recvListen = new RecvListen(listenSockfd, pool);
  pool->append(recvListen);

  while (true) {
    int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
    if ((number < 0) && (errno != EINTR)) {
      console->error("epoll failure\n");
      break;
    }
    for (int i = 0; i < number; i++) {
      int sockfd = events[i].data.fd;
      if (sockfd == masterfd) {
        if (events[i].events & EPOLLIN) {
          masterSockManger.recvMsg();
          masterSockManger.getRecvBuf(recvbuf, recvbuflen);
          coflowSimClient_logger->info("recbuf: {}", recvbuf);
          int insstart = 0, insend = 0;
          for (int i = 0; i < recvbuflen; i++) {
            if (recvbuf[i] == '(') {
              insstart = i + 1;
            }

            if (recvbuf[i] == ')') {
              insend = i;
              coflowSimClient_logger->info("ins: ({})", recvbuf + insstart);
              SendFile *sendFile = new SendFile(
                  recvbuf + insstart, insend - insstart, masterSockManger);
              sendFiles.push_back(sendFile);
              pool->append(sendFile);
            }
          }
          if (insend < insstart) {
            int i = 0;
            recvbuf[i++] = '(';
            while (insstart < recvbuflen) {
              recvbuf[i++] = recvbuf[insstart++];
            }
            recvbuf[i] = '\0';
            recvbuflen = i;
          } else {
            recvbuf[0] = '\0';
            recvbuflen = 0;
          }
        } else if (events[i].events & EPOLLOUT) {
          // cout << "EPOLLOUT" << endl;
          // cout << "EPOLLOUT_end" << endl;
        }

      } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        //                sockManger.closeConn(sockfd);
      } else {
      }
    }
  }
}

bool parseConfig(char const *configFilePath) {
  auto console = spdlog::stdout_color_mt("parseConfig");
  FILE *fp = fopen(configFilePath, "r");
  char readBuffer[65536];
  FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  Document document;
  document.ParseStream(is);
  fclose(fp);

  stringstream ss;

  assert(document.HasMember("server_ip"));
  assert(document["server_ip"].IsString());
  ss << document["server_ip"].GetString();
  ss >> serverIP;
  ss.clear();
  console->info("server_ip is {}", serverIP);

  assert(document.HasMember("server_port"));
  assert(document["server_port"].IsInt());
  ss << document["server_port"].GetInt();
  ss >> serverPort;
  ss.clear();
  console->info("server_port is {}", serverPort);

  assert(document.HasMember("thread_num"));
  assert(document["thread_num"].IsInt());
  ss << document["thread_num"].GetInt();
  ss >> threadNum;
  ss.clear();
  console->info("thread_num is {}", threadNum);
  return true;
}