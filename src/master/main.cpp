#include <arpa/inet.h>
#include <assert.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <strings.h>
#include <sys/epoll.h>
#include <unordered_map>

#include "lib/epollFunctions.h"
#include "lib/threadclass.h"
#include "lib/threadpool.h"
#include "scheduler/scheduler.h"
#include "socket/socketManage.h"
#include "traceProducer/CoflowBenchmarkTraceProducer.h"
#include "traceProducer/producer.h"

//#include "spdlog/spdlog.h"
//
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
using namespace rapidjson;

#define BACKLOG 10

#define MAX_EVENT_NUMBER 10000

char serverIP[64] = "127.0.0.1";
int serverPort = 4002;
char broadcastIP[64] = "255.255.255.255";
int broadcastPort = 4001;
char FBfilePath[1024] = "../res/FB2010-1Hr-150-0.txt";
int threadNum = 8;
bool parseConfig(char const *configFilePath) {
  //    ifstream configFin;
  //    configFin.open(configFilePath);
  FILE *fp = fopen(configFilePath, "r"); // 非 Windows 平台使用 "r"
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

  assert(document.HasMember("server_port"));
  assert(document["server_port"].IsInt());
  ss << document["server_port"].GetInt();
  ss >> serverPort;
  ss.clear();

  assert(document.HasMember("broadcast_ip"));
  assert(document["broadcast_ip"].IsString());
  ss << document["broadcast_ip"].GetString();
  ss >> broadcastIP;
  ss.clear();

  assert(document.HasMember("broadcast_port"));
  assert(document["broadcast_port"].IsInt());
  ss << document["broadcast_port"].GetInt();
  ss >> broadcastPort;
  ss.clear();

  assert(document.HasMember("FBfilePath"));
  assert(document["FBfilePath"].IsString());
  ss << configFilePath;
  ss >> FBfilePath;
  ss.clear();
  int index = strlen(FBfilePath) - 1;
  while (index >= 0 && FBfilePath[index] != '/')
    index--;
  FBfilePath[++index] = '\0';
  ss << FBfilePath;
  ss << document["FBfilePath"].GetString();
  ss >> FBfilePath;
  ss.clear();

  assert(document.HasMember("thread_num"));
  assert(document["thread_num"].IsInt());
  ss << document["thread_num"].GetInt();
  ss >> threadNum;
  ss.clear();

  return true;
}
int test() {
  //    char listenip[50];
  //    int listenport;
  //    fstream Configfin;
  //    Configfin.open(CONFIG_PATH);
  //    Configfin >> listenip;
  //    Configfin >> listenport;
  //    Configfin.close();
  //    printf("ip: %s port: %d \n", listenip, listenport);
  int bcSockfd, listenSockfd;
  struct sockaddr_in bcAddr, listenAddr;
  ThreadPool<ThreadClass> *pool = nullptr;

  // 初始化广播socket和地址
  if ((bcSockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
    printf("error: Creating bcspcket fail\n");
    return -1;
  }
  int opval = 1;
  setsockopt(bcSockfd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &opval,
             sizeof(int));
  memset(&bcAddr, 0, sizeof(struct sockaddr_in));
  bcAddr.sin_family = AF_INET;
  bcAddr.sin_family = AF_INET;
  bcAddr.sin_addr.s_addr = inet_addr(broadcastIP);
  bcAddr.sin_port = htons(broadcastPort);

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
  listenAddr.sin_port = htons(serverPort);
  if (bind(listenSockfd, (struct sockaddr *)&listenAddr,
           sizeof(struct sockaddr)) == -1) {
    perror("Bind error\n");
    return -1;
  }
  if (listen(listenSockfd, BACKLOG) == -1) {
    perror("listen() error\n");
    return -1;
  }

  // 线程池
  try {
    pool = new ThreadPool<ThreadClass>;
  } catch (...) {
    return 1;
  }

  CoflowBenchmarkTraceProducer producer(FBfilePath, "123");
  vector<Coflow *> *coflows = new vector<Coflow *>;
  producer.prepareCoflows(coflows);
  coflows->at(1)->toString();

  scheduler *scheduler1 = new scheduler();
  machineManager *machineManager1 = new machineManager();
  machineManager1->setLogicMachineNum(150);
  scheduler1->setMachines(machineManager1);
  scheduler1->setUnregisterCoflows(coflows);
  pool->append(scheduler1);

  epoll_event events[MAX_EVENT_NUMBER];
  int epollfd = epoll_create(5);
  assert(epollfd != -1);
  addfd(epollfd, listenSockfd, false);
  SocketManage::sEpollfd = epollfd;

  char bcmsg[50] = "127.0.0.1 4002";
  //    fstream Cofigfin;
  //    Cofigfin.open(CONFIG_PATH);
  //    Cofigfin.getline(bcmsg, 50);
  //    Cofigfin.close();

  while (true) {

    int sendBytes;
    if ((sendBytes = sendto(bcSockfd, bcmsg, strlen(bcmsg), 0,
                            (struct sockaddr *)&bcAddr,
                            sizeof(struct sockaddr))) == -1) {
      printf("sendto fail, errno=%d\n", errno);
      return -1;
    }
    printf("msg=%s, msgLen=%d, sendBytes=%d\n", bcmsg, strlen(bcmsg),
           sendBytes);

    int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
    printf("epoll_number: %d\n", number);
    if ((number < 0) && (errno != EINTR)) {
      printf("epoll failure\n");
      break;
    }
    for (int i = 0; i < number; i++) {
      int sockfd = events[i].data.fd;
      if (sockfd == listenSockfd) {
        struct sockaddr_in client_address;
        socklen_t client_addrlength = sizeof(client_address);
        int connfd = accept(listenSockfd, (struct sockaddr *)&client_address,
                            &client_addrlength);
        if (connfd < 0) {
          printf("errno is: %d\n", errno);
          continue;
        }
        printf("accept connfd: %d\n", connfd);
        machineManager1->addOnePhysicsMachine(connfd, connfd, client_address);
        printf(
            "machineID: %d\n",
            machineManager1->getPhyMachineByMachineID(connfd)->getMachineID());
      } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        machineManager1->removeOnePhysicsMachine(sockfd);
      } else if ((events[i].events & EPOLLIN) ||
                 (events[i].events & EPOLLOUT)) {
        pool->append(machineManager1->getPhyMachineByMachineID(sockfd));
      } else {
      }
    }
  }
}

int main(int argc, char const *argv[]) {

  std::cout << "Hello, World!" << argv[1] << std::endl;
  parseConfig(argv[1]);

  printf("FBfilePath = %s\n", FBfilePath);
  //    test();
  return 0;
}