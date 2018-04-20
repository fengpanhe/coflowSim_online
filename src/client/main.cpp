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
#include "RecvManager.h"
#include "SendManager.h"
using namespace rapidjson;
using namespace std;
#define CONN_PORT 4002

#define BACKLOG 65535
#define MAX_EVENT_NUMBER 10000

char master_server_ip[64] = "127.0.0.1";
int master_server_port = 4003;
char client_server_ip[64] = "127.0.0.1";
int client_server_port = 4003;
char net_card_name[256] = "eth0";
double net_card_bandwidth_MBs = 100;
int thread_num = 8;
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

//  ThreadPool *sender_pool = nullptr;
//  ThreadPool *receiver_pool = nullptr;
  ThreadPool *pool = nullptr;
  int listenSockfd;
  struct sockaddr_in listenAddr {};

  // epoll创建
  epoll_event events[MAX_EVENT_NUMBER];
  int epollfd = epoll_create(MAX_EVENT_NUMBER);
  assert(epollfd != -1);
  SocketManage::sEpollfd = epollfd; // SocketManage类的静态变量赋值
  console->info("epoll_event is created!");
  // masterSockManger的创建，接收master的连接
  SocketManage masterSockManger;

  // 初始化监听socket
  if ((listenSockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Creating listenSocket failed");
    return -1;
  }
  struct linger tmp = {1, 0};
  setsockopt(listenSockfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
  bzero(&listenAddr, sizeof(listenAddr));
  listenAddr.sin_family = AF_INET;
  inet_pton(AF_INET, master_server_ip, &listenAddr.sin_addr);
  listenAddr.sin_port = htons(static_cast<uint16_t>(master_server_port));
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
  struct sockaddr_in master_address {};
  socklen_t master_addrlength = sizeof(master_address);
  console->info("Waiting for master...");
  int masterfd = accept(listenSockfd, (struct sockaddr *)&master_address,
                        &master_addrlength);
  masterSockManger.initSocket(masterfd, master_address);
  console->info("masterSockManger is ready!");

  // 创建线程池
//  int half_thread_number = thread_num > 4 ? thread_num / 2 : 2;
  try {
//    sender_pool = new ThreadPool(half_thread_number);
//    receiver_pool = new ThreadPool(half_thread_number);
    pool = new ThreadPool(thread_num);
  } catch (...) {
    return 1;
  }

  printf("debug0\n");
  auto tc_manager = new TrafficControlManager(net_card_name, net_card_bandwidth_MBs);
  printf("debug1\n");
  auto *recv_manager = new RecvManager(pool, listenSockfd);
  auto *send_manager = new SendManager(tc_manager, pool, &masterSockManger);
  printf("debug2\n");
  pool->append(recv_manager);
  printf("debug3\n");
  pool->append(send_manager);
  printf("debug4\n");

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
              send_manager->appendTask(recvbuf + insstart, insend - insstart);
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

  assert(document.HasMember("master_server_ip"));
  assert(document["master_server_ip"].IsString());
  ss << document["master_server_ip"].GetString();
  ss >> master_server_ip;
  ss.clear();
  console->info("master_server_ip is {}", master_server_ip);

  assert(document.HasMember("master_server_port"));
  assert(document["master_server_port"].IsInt());
  ss << document["master_server_port"].GetInt();
  ss >> master_server_port;
  ss.clear();
  console->info("master_server_port is {}", master_server_port);

  assert(document.HasMember("client_server_ip"));
  assert(document["client_server_ip"].IsString());
  ss << document["client_server_ip"].GetString();
  ss >> client_server_ip;
  ss.clear();
  console->info("client_server_ip is {}", client_server_ip);

  assert(document.HasMember("client_server_port"));
  assert(document["client_server_port"].IsInt());
  ss << document["client_server_port"].GetInt();
  ss >> client_server_port;
  ss.clear();
  console->info("client_server_port is {}", client_server_port);

  assert(document.HasMember("thread_num"));
  assert(document["thread_num"].IsInt());
  ss << document["thread_num"].GetInt();
  ss >> thread_num;
  ss.clear();
  console->info("thread_num is {}", thread_num);

  assert(document.HasMember("net_card_name"));
  assert(document["net_card_name"].IsString());
  ss << document["net_card_name"].GetString();
  ss >> net_card_name;
  ss.clear();
  console->info("net_card_name is {}", net_card_name);

  assert(document.HasMember("net_card_bandwidth_MBs"));
  assert(document["net_card_bandwidth_MBs"].IsDouble());
  ss << document["net_card_bandwidth_MBs"].GetDouble();
  ss >> net_card_bandwidth_MBs;
  ss.clear();
  console->info("net_card_bandwidth_MBs is {}", net_card_bandwidth_MBs);
  return true;
}