#include "lib/epollFunctions.h"
#include "lib/threadpool.h"
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
using namespace rapidjson;
using namespace std;
#define BROADCAST_LISTEN_PORT 4001
#define CONN_PORT 4002

#define BACKLOG 65535
#define MAX_EVENT_NUMBER 10000

char serverIP[64] = "127.0.0.1";
int serverPort = 4003;
int threadNum = 8;

char *recvbuf = new char[4096];
int recvbuflen = 0;

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

int coflowSimClient() {
  auto console = spdlog::stdout_color_mt("coflowSimClient");
  auto coflowSimClient_logger =
      spdlog::basic_logger_mt("coflowSimClient_logger", "coflowSimClient.log");

  ThreadPool<ThreadClass> *pool = nullptr;
  try {
    pool = new ThreadPool<ThreadClass>(threadNum);
  } catch (...) {
    return 1;
  }
  //    int bcListenSockfd, connSockfd;
  struct sockaddr_in bclAddr, connAddr;
  int bcSockfd, listenSockfd;
  struct sockaddr_in bcAddr {
  }, listenAddr{};

  /*/  if ((bcListenSockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
  //    printf("socket fail\n");
  //    return -1;
  //  }
  //  int set = 1;
  //  setsockopt(bcListenSockfd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));
  //  memset(&bclAddr, 0, sizeof(struct sockaddr_in));
  //  bclAddr.sin_family = AF_INET;
  //  bclAddr.sin_port = htons(BROADCAST_LISTEN_PORT);
  //  bclAddr.sin_addr.s_addr = INADDR_ANY;
  //  if (bind(bcListenSockfd, (struct sockaddr *)&bclAddr,
  //           sizeof(struct sockaddr)) == -1) {
  //    printf("bind fail\n");
  //    return -1;
  //  }
  //  int recvbytes;
  //  char recvbuf[128];
  //  socklen_t addrLen = sizeof(struct sockaddr_in);
  //  if ((recvbytes = recvfrom(bcListenSockfd, recvbuf, 128, 0,
  //                            (struct sockaddr *)&bclAddr, &addrLen)) != -1) {
  //    recvbuf[recvbytes] = '\0';
  //    printf("receive a broadCast messgse:%s\n", recvbuf);
  //  } else {
  //    printf("recvfrom fail\n");
  //  }
  //  close(bcListenSockfd);
  //
  //  stringstream ss(recvbuf);
  //  char ip[20];
  //  int port;
  //  ss >> ip;
  //  ss >> port;
  //  printf("ip: %s, port: %d\n", ip, port);
  //  memset(&connAddr, 0, sizeof(struct sockaddr_in));
  //  connAddr.sin_family = AF_INET;
  //  connAddr.sin_addr.s_addr = inet_addr(ip);
  //  connAddr.sin_port = htons(port);
  //  if ((connSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
  //    printf("Failed to create socket");
  //  }
  //
  //  if (connect(connSockfd, (struct sockaddr *)&connAddr, sizeof(connAddr)) <
  0) {
  //    printf("Failed to connect with server");
  //  }
  //  int error = 0;
  //  socklen_t len = sizeof(error);
  //  getsockopt(connSockfd, SOL_SOCKET, SO_ERROR, &error, &len);
  //  int reuse = 1;
  //  setsockopt(connSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  */

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

  //    SocketManage sockManger;

  // epoll创建
  epoll_event events[MAX_EVENT_NUMBER];
  int epollfd = epoll_create(MAX_EVENT_NUMBER);
  assert(epollfd != -1);
  SocketManage::sEpollfd = epollfd;

  console->info("epoll_event is created!");

  //    masterSockManger的创建，接收master的连接
  SocketManage masterSockManger;
  struct sockaddr_in master_address {};
  socklen_t master_addrlength = sizeof(master_address);
  console->info("Waiting for master...");
  int masterfd = accept(listenSockfd, (struct sockaddr *)&master_address,
                        &master_addrlength);
  masterSockManger.initSocket(masterfd, master_address);

  console->info("masterSockManger is ok!");

  vector<SendFile *> sendFiles;
  ReceFile *receFiles = new ReceFile[65536];

//  addfd(epollfd, listenSockfd, false);
  RecvListen *recvListen = new RecvListen(listenSockfd, pool);
  pool->append(recvListen);
  int f_int = 0;
  while (true) {
    int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
    if ((number < 0) && (errno != EINTR)) {
      console->error("epoll failure\n");
      break;
    }
    for (int i = 0; i < number; i++) {
      int sockfd = events[i].data.fd;
      if (sockfd == listenSockfd) {

//        struct sockaddr_in client_address {};
//        socklen_t client_addrlength = sizeof(client_address);
//        int connfd = accept(listenSockfd, (struct sockaddr *)&client_address,
//                            &client_addrlength);
//        if (connfd < 0) {
//          continue;
//        }
//        receFiles[connfd].file_int = f_int;
//        f_int++;
//        receFiles[connfd].initSocket(connfd, client_address);
//        pool->append(receFiles + connfd);
      } else if (sockfd == masterfd) {

        if (events[i].events & EPOLLIN) {
          masterSockManger.recvMsg();
          masterSockManger.getRecvBuf(recvbuf, recvbuflen);
          // cout << "recvbuf" << recvbuf << endl;
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
          cout << "EPOLLOUT" << endl;

          cout << "EPOLLOUT_end" << endl;
        }

      } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        //                sockManger.closeConn(sockfd);
      } else {
      }
    }

    /*        for (auto& it:sendFiles) {
                if (it->reflag && it->coflowID>-1) {

                    char tmpstr[100] = "a";
    //                memset(tmpstr,'0',sizeof(tmpstr));
                    stringstream ss;
                    ss << it->coflowID;
                    ss >> tmpstr;
                    ss.clear();

                    tmpstr[strlen(tmpstr)] = ' ';

                    ss << it->flowID;
                    ss >> tmpstr+strlen(tmpstr);
                    ss.clear();

                    tmpstr[strlen(tmpstr)] = ' ';

                    ss << it->endTime;
                    ss >> tmpstr+strlen(tmpstr);
                    ss.clear();
                    cout << tmpstr << endl;
                    masterSockManger.setSendMsg(tmpstr, strlen(tmpstr));
                    masterSockManger.run();
                    it->reflag = false;
                    break;
                }
            }
    /*        //    int dayle = 100000000000;
            //    while(true){
            //        dayle = 1000000000;
            //        while(--dayle > 0);
            //        sockManger.run();
            //    }
            //    printf("%s\n", "connected");
            //    int dayle = 100000000000;
            //    int recvlen = 0;
            //    while (true) {
            //        int recvlen = 0;
            //        int bytes_read = 0;
            //        char buf[1024];
            //        while (true) {
            ////            dayle = 1000000000;
            ////            while(--dayle > 0);
            //            bytes_read = static_cast<int>(recv(connSockfd, buf +
    recvlen,
            //                                               10, 0));
            //            if (bytes_read == -1) {
            //                if (errno == EAGAIN  | errno == EWOULDBLOCK) {
            //                    printf("break");
            //                    break;
            //                }
            //                printf("Failed");
            //                return false;
            //            } else if (bytes_read == 0) {
            //                printf("Failed");
            //                return false;
            //            }
            //            recvlen += bytes_read;
            //            if(recvlen >= 5) break;
            //            printf("buf: %s  len: %d\n", buf, recvlen);
            //        }
            //        for (int i = 0; i < recvlen; ++i) {
            //            cout << buf[i];
            //        }
            //        printf("\n");
            //    } */
  }
}

namespace spd = spdlog;
int main(int argc, char const *argv[]) {
  auto console = spd::stdout_color_mt("console");
  parseConfig(argv[1]);
  coflowSimClient();
  return 0;
}