#include <iostream>
#include <unordered_map>
#include <assert.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/epoll.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include "traceProducer/producer.h"
#include "traceProducer/CoflowBenchmarkTraceProducer.h"
#include "lib/threadpool.h"
#include "lib/thread.h"
#include "lib/epollFunctions.h"
#include "socket/socketManage.h"
#include "scheduler/scheduler.h"

#define BROADCAST_IP "255.255.255.255"
#define BROADCAST_PORT 4001

#define LISTENED_IP "127.0.0.1"
#define LISTENED_PORT 4002
#define BACKLOG 10

#define MAX_EVENT_NUMBER 10000
//const char* ip = "127.0.0.1";
//int port = 12345;
#define CONFIG_PATH "../config"

int test(){
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
    threadpool<thread>* pool = nullptr;

    // 初始化广播socket和地址
    if ((bcSockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1){
        printf("error: Creating bcspcket fail\n");
        return -1;
    }
    int opval = 1;
    setsockopt(bcSockfd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &opval, sizeof(int));
    memset(&bcAddr, 0, sizeof(struct sockaddr_in));
    bcAddr.sin_family = AF_INET;
    bcAddr.sin_family = AF_INET;
    bcAddr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    bcAddr.sin_port = htons(BROADCAST_PORT);


    // 初始化监听socket
    if ((listenSockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Creating listenSocket failed");
        return -1;
    }
    struct linger tmp = { 1, 0 };
    setsockopt(listenSockfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    bzero( &listenAddr, sizeof( listenAddr ) );
    listenAddr.sin_family = AF_INET;
    inet_pton(AF_INET, LISTENED_IP, &listenAddr.sin_addr);
    listenAddr.sin_port = htons(LISTENED_PORT);
    if (bind(listenSockfd, (struct sockaddr *)&listenAddr, sizeof(struct sockaddr)) == -1) {
        perror("Bind error\n");
        return -1;
    }
    if (listen(listenSockfd, BACKLOG) == -1) {
        perror("listen() error\n");
        return -1;
    }

    // 线程池
    try{
        pool = new threadpool<thread>;
    } catch(...) {
        return 1;
    }


    CoflowBenchmarkTraceProducer producer("./FB2010-1Hr-150-0.txt", "123");
    vector<Coflow*>* coflows = new vector<Coflow*>;
    producer.prepareCoflows(coflows);
    coflows->at(1)->toString();

    scheduler * scheduler1 = new scheduler();
    machineManager* machineManager1 = new machineManager();
    machineManager1->setLogicMachineNum(150);
    scheduler1->setMachines(machineManager1);
    scheduler1->setUnregisterCoflows(coflows);
    pool->append(scheduler1);

    epoll_event events[ MAX_EVENT_NUMBER ];
    int epollfd = epoll_create( 5 );
    assert( epollfd != -1 );
    addfd( epollfd, listenSockfd, false );
    SocketManage::sEpollfd = epollfd;

    char bcmsg[50] = "127.0.0.1 4002";
//    fstream Cofigfin;
//    Cofigfin.open(CONFIG_PATH);
//    Cofigfin.getline(bcmsg, 50);
//    Cofigfin.close();

    while(true){

        int sendBytes;
        if ((sendBytes = sendto(bcSockfd, bcmsg, strlen(bcmsg), 0, (struct sockaddr *)&bcAddr, sizeof(struct sockaddr))) == -1) {
            printf("sendto fail, errno=%d\n", errno);
            return -1;
        }
        printf("msg=%s, msgLen=%d, sendBytes=%d\n", bcmsg, strlen(bcmsg), sendBytes);


        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );
        printf("epoll_number: %d\n", number);
        if ( ( number < 0 ) && ( errno != EINTR ) ) {
            printf( "epoll failure\n" );
            break;
        }
        for ( int i = 0; i < number; i++ ) {
            int sockfd = events[i].data.fd;
            if( sockfd == listenSockfd ) {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenSockfd, ( struct sockaddr* )&client_address, &client_addrlength );
                if ( connfd < 0 ) {
                    printf( "errno is: %d\n", errno );
                    continue;
                }
                printf("accept connfd: %d\n", connfd);
                machineManager1->addOnePhysicsMachine(connfd, connfd, client_address);
                printf("machineID: %d\n", machineManager1->getPhyMachineByMachineID(connfd)->getMachineID());
            }
            else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {
                machineManager1->removeOnePhysicsMachine(sockfd);
            }
            else if( (events[i].events & EPOLLIN) || (events[i].events & EPOLLOUT)) {
                pool->append(machineManager1->getPhyMachineByMachineID(sockfd));
            }
            else {}
        }
    }
}

int main() {
    std::cout << "Hello, World!" << std::endl;


    test();
    return 0;
}