#include "lib/epollFunctions.h"
#include "socket/socketManage.h"
#include <arpa/inet.h>
#include <assert.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/epoll.h>
#include <zconf.h>
#include "lib/threadpool.h"
#include "spdlog/spdlog.h"
#include "sendFile.h"
#include "receFile.h"
using namespace std;
#define BROADCAST_LISTEN_PORT 4001
#define CONN_PORT 4002

#define BACKLOG 10

char serverIP[64] = "127.0.0.1";
int serverPort = 4003;
int test()
{
    ThreadPool<ThreadClass>* pool = nullptr;
    try {
        pool = new ThreadPool<ThreadClass>(8);
    }
    catch (...) {
        return 1;
    }
    int bcListenSockfd, connSockfd;
    struct sockaddr_in bclAddr, connAddr;
    int bcSockfd, listenSockfd;
    struct sockaddr_in bcAddr{
    }, listenAddr{};
//  if ((bcListenSockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
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
//  if (connect(connSockfd, (struct sockaddr *)&connAddr, sizeof(connAddr)) < 0) {
//    printf("Failed to connect with server");
//  }
//  int error = 0;
//  socklen_t len = sizeof(error);
//  getsockopt(connSockfd, SOL_SOCKET, SO_ERROR, &error, &len);
//  int reuse = 1;
//  setsockopt(connSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 初始化监听socket
    if ((listenSockfd = socket(AF_INET, SOCK_STREAM, 0))==-1) {
        perror("Creating listenSocket failed");
        return -1;
    }
    struct linger tmp = {1, 0};
    setsockopt(listenSockfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    bzero(&listenAddr, sizeof(listenAddr));
    listenAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP, &listenAddr.sin_addr);
    listenAddr.sin_port = htons(static_cast<uint16_t>(serverPort));
    if (bind(listenSockfd, (struct sockaddr*) &listenAddr,
            sizeof(struct sockaddr))==-1) {
        perror("Bind error\n");
        return -1;
    }
    if (listen(listenSockfd, BACKLOG)==-1) {
        perror("listen() error\n");
        return -1;
    }

    SocketManage sockManger;

    epoll_event events[5];
    int epollfd = epoll_create(5);
    assert(epollfd!=-1);
    //    addfd( epollfd, listenSockfd, false );
    SocketManage::sEpollfd = epollfd;
    sockManger.initSocket(connSockfd, connAddr);

    SocketManage masterSockManger;
    struct sockaddr_in master_address{};
    socklen_t master_addrlength = sizeof(master_address);
    int masterfd = accept(listenSockfd, (struct sockaddr*) &master_address,
            &master_addrlength);
    masterSockManger.initSocket(masterfd, master_address);

    vector<SendFile*> sendFiles;
    ReceFile* receFiles = new ReceFile[65536];
    printf("connSockfd: %d\n", connSockfd);

    addfd(epollfd, listenSockfd, false);
    while (true) {
        int number = epoll_wait(epollfd, events, 5, -1);
//        printf("epoll_number: %d\n", number);
        if ((number<0) && (errno!=EINTR)) {
            printf("epoll failure\n");
            break;
        }
        for (int i = 0; i<number; i++) {
            int sockfd = events[i].data.fd;
            if (sockfd==listenSockfd) {
                struct sockaddr_in client_address{};
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenSockfd, (struct sockaddr*) &client_address,
                        &client_addrlength);
                if (connfd<0) {
                    continue;
                }
                receFiles[connfd].initSocket(connfd, client_address);
            }
            else if (sockfd == masterfd) {
                masterSockManger.run();

                char * recvbuf = new char[1024];
                int buflen = 0;
                masterSockManger.getRecvBuf(recvbuf, buflen);
                SendFile* sendFile = new SendFile(recvbuf);
                sendFiles.push_back(sendFile);
                pool->append(sendFile);
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                sockManger.closeConn(sockfd);
            }
            else if ((events[i].events & EPOLLIN) ||
                    (events[i].events & EPOLLOUT)) {
                pool->append(receFiles + sockfd);
//                receFiles[sockfd].run();
            }
            else {
            }

        }

        for(auto & it:sendFiles){
            if(it->reflag && it->coflowID > -1){

                char tmpstr[100] = "a";
//                memset(tmpstr,'0',sizeof(tmpstr));
                stringstream ss;
                ss << it->coflowID;
                ss >> tmpstr;
                ss.clear();

                tmpstr[strlen(tmpstr)] = ' ';

                ss << it->flowID;
                ss >> tmpstr + strlen(tmpstr);
                ss.clear();

                tmpstr[strlen(tmpstr)] = ' ';

                ss << it->endTime;
                ss >> tmpstr + strlen(tmpstr);
                ss.clear();
                cout << tmpstr << endl;
                masterSockManger.setSendMsg(tmpstr, strlen(tmpstr));
                masterSockManger.run();
                it->reflag = false;
                break;
            }
        }
        //    int dayle = 100000000000;
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
        //            bytes_read = static_cast<int>(recv(connSockfd, buf + recvlen,
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
        //    }
    }
}

namespace spd = spdlog;
int main()
{
    auto console = spd::stdout_color_mt("console");
    console->info("Welcome to spdlog!");
    console->error("Some error message with arg{}..", 1);
    std::cout << "Hello, World!" << std::endl;
    test();
    return 0;
}