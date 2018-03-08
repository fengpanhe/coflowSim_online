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
#include "scheduler/Scheduler.h"
#include "socket/socketManage.h"
#include "traceProducer/CoflowBenchmarkTraceProducer.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "spdlog/spdlog.h"
using namespace rapidjson;

#define BACKLOG 10

#define MAX_EVENT_NUMBER 10000

char serverIP[64] = "127.0.0.1";
int serverPort = 4002;
char broadcastIP[64] = "255.255.255.255";
int broadcastPort = 4001;
char FBfilePath[1024] = "../res/FB2010-1Hr-150-0.txt";
char machine_define_path[1024] = "../res/machine_define.json";
int threadNum = 8;

bool parseConfig(char const* configFilePath)
{
    auto console = spdlog::stdout_color_mt("parseConfig");
    //    ifstream configFin;
    //    configFin.open(configFilePath);
    FILE* fp = fopen(configFilePath, "r"); // 非 Windows 平台使用 "r"
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

    assert(document.HasMember("broadcast_ip"));
    assert(document["broadcast_ip"].IsString());
    ss << document["broadcast_ip"].GetString();
    ss >> broadcastIP;
    ss.clear();
    console->info("broadcast_ip is {}", broadcastIP);

    assert(document.HasMember("broadcast_port"));
    assert(document["broadcast_port"].IsInt());
    ss << document["broadcast_port"].GetInt();
    ss >> broadcastPort;
    ss.clear();
    console->info("broadcast_port is {}", broadcastPort);

    assert(document.HasMember("FBfilePath"));
    assert(document["FBfilePath"].IsString());
    ss << configFilePath;
    ss >> FBfilePath;
    ss.clear();
    auto index = static_cast<int>(strlen(FBfilePath)-1);
    while (index>=0 && FBfilePath[index]!='/')
        index--;
    FBfilePath[++index] = '\0';
    ss << FBfilePath;
    ss << document["FBfilePath"].GetString();
    ss >> FBfilePath;
    ss.clear();
    console->info("FBfilePath is {}", FBfilePath);

    assert(document.HasMember("machine_define_path"));
    assert(document["machine_define_path"].IsString());
    ss << configFilePath;
    ss >> machine_define_path;
    ss.clear();
    index = static_cast<int>(strlen(machine_define_path)-1);
    while (index>=0 && machine_define_path[index]!='/')
        index--;
    machine_define_path[++index] = '\0';
    ss << machine_define_path;
    ss << document["machine_define_path"].GetString();
    ss >> machine_define_path;
    ss.clear();
    console->info("machine_define_path is {}", machine_define_path);

    assert(document.HasMember("thread_num"));
    assert(document["thread_num"].IsInt());
    ss << document["thread_num"].GetInt();
    ss >> threadNum;
    ss.clear();
    console->info("thread_num is {}", threadNum);

    return true;
}
//,
bool parseMachineDenfine(char const* machineDefinePath, MachineManager*& machineManager)
{
    FILE* fp = fopen(machineDefinePath, "r"); // 非 Windows 平台使用 "r"
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    Document document;
    document.ParseStream(is);
    fclose(fp);

    stringstream ss;

    char master_ip[64];
    int master_port;
    assert(document.HasMember("master_ip"));
    assert(document["master_ip"].IsString());
    ss << document["master_ip"].GetString();
    ss >> master_ip;
    ss.clear();

    assert(document.HasMember("master_port"));
    assert(document["master_port"].IsInt());
    ss << document["master_port"].GetInt();
    ss >> master_port;
    ss.clear();

    const Value& clients_ip = document["clients_ip"];
    const Value& clients_port = document["clients_port"];

    assert(clients_ip.IsArray());
    assert(clients_port.IsArray());
    for (SizeType i = 0; i<clients_ip.Size(); i++) {
        char client_ip[64];
        int client_port;

        ss << clients_ip[i].GetString();
        ss >> client_ip;
        ss.clear();

        ss << clients_port[i].GetInt();
        ss >> client_port;
        ss.clear();

        struct sockaddr_in connAddr;
        int connSockfd;
        memset(&connAddr, 0, sizeof(struct sockaddr_in));
        connAddr.sin_family = AF_INET;
        connAddr.sin_addr.s_addr = inet_addr(client_ip);
        connAddr.sin_port = htons(client_port);
        if ((connSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0) {
            printf("Failed to create socket");
        }

        if (connect(connSockfd, (struct sockaddr*) &connAddr, sizeof(connAddr))<0) {
            printf("Failed to connect with server");
        }
        int error = 0;
        socklen_t len = sizeof(error);
        getsockopt(connSockfd, SOL_SOCKET, SO_ERROR, &error, &len);
        int reuse = 1;
        setsockopt(connSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        machineManager->addOnePhysicsMachine(connSockfd, client_ip, client_port, connAddr);
    }

}
int test()
{
    auto console = spdlog::stdout_color_mt("console");

    int bcSockfd, listenSockfd;
    struct sockaddr_in bcAddr{
    }, listenAddr{};
    ThreadPool<ThreadClass>* pool = nullptr;

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd!=-1);
    addfd(epollfd, listenSockfd, false);
    SocketManage::sEpollfd = epollfd;

    // 初始化广播socket和地址
    if ((bcSockfd = socket(PF_INET, SOCK_DGRAM, 0))==-1) {
        perror("Creating bcsocket failed\n");
        return -1;
    }
    int opval = 1;
    setsockopt(bcSockfd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &opval,
            sizeof(int));
    memset(&bcAddr, 0, sizeof(struct sockaddr_in));
    bcAddr.sin_family = AF_INET;
    bcAddr.sin_family = AF_INET;
    bcAddr.sin_addr.s_addr = inet_addr(broadcastIP);
    bcAddr.sin_port = htons(static_cast<uint16_t>(broadcastPort));
    console->info("Successfully initialized bcSockfd and address!");

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
    console->info("Successfully initialized listenSockfd and address!");

    // 线程池
    try {
        pool = new ThreadPool<ThreadClass>(threadNum);
    }
    catch (...) {
        return 1;
    }

    CoflowBenchmarkTraceProducer producer(FBfilePath, "123");
    auto* coflows = new vector<Coflow*>;
    producer.prepareCoflows(coflows);
    //    coflows->at(1)->toString();

    auto* scheduler1 = new Scheduler();
    auto* machineManager1 = new MachineManager();
    machineManager1->setLogicMachineNum(150);
    parseMachineDenfine(machine_define_path, machineManager1);

    scheduler1->setMachines(machineManager1);
    scheduler1->setCoflows(coflows);
    pool->append(scheduler1);

    char bcmsg[50] = "127.0.0.1 4002";
    while (true) {

//    if (sendto(bcSockfd, bcmsg, strlen(bcmsg), 0, (struct sockaddr *)&bcAddr,
//               sizeof(struct sockaddr)) == -1) {
//      console->error("sendto fail, errno={}\n", errno);
//      return -1;
//    }
//    console->info("Broadcast a message: {}", bcmsg);

        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((number<0) && (errno!=EINTR)) {
            console->error("epoll failure\n");
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
                    console->error("errno is: {}\n", errno);
                    continue;
                }
                console->info("accept connfd: {}", connfd);
//        machineManager1->addOnePhysicsMachine(connfd, connfd, client_address);
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                machineManager1->removeOnePhysicsMachine(sockfd);
            }
            else if ((events[i].events & EPOLLIN) ||
                    (events[i].events & EPOLLOUT)) {
                pool->append(machineManager1->getPhyMachineByMachineID(sockfd));
            }
            else {
            }
        }
    }
}

int main(int argc, char const* argv[])
{

    //    std::cout << "Hello, World!" << argv[1] << std::endl;
    parseConfig(argv[1]);

    test();
    int now1 = time(0);
    printf("时间： %d \n", now1);
    return 0;
}