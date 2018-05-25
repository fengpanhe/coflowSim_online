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
//#include "lib/threadclass.h"
#include "lib/ThreadPool.h"
#include "scheduler/Scheduler.h"
#include "socket/socketManage.h"
#include "traceProducer/CoflowBenchmarkTraceProducer.h"

#include "handler/CoflowJsonHandler.h"
#include "handler/IndexHandler.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"
#include "spdlog/spdlog.h"
#include "webserver/WebServer.h"
#include <rapidjson/writer.h>
using namespace rapidjson;

#define BACKLOG 65535

#define MAX_EVENT_NUMBER 10000

char master_server_ip[64] = "127.0.0.1";
int master_server_port = 4002;
char broadcastIP[64] = "255.255.255.255";
int broadcastPort = 4001;
char FBfilePath[1024] = "../res/FB2010-1Hr-150-0.txt";
char machine_define_path[1024] = "../res/machine_define.json";
int thread_num = 8;

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
  ss >> master_server_ip;
  ss.clear();
  console->info("server_ip is {}", master_server_ip);

  assert(document.HasMember("server_port"));
  assert(document["server_port"].IsInt());
  ss << document["server_port"].GetInt();
  ss >> master_server_port;
  ss.clear();
  console->info("server_port is {}", master_server_port);

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
  auto index = static_cast<int>(strlen(FBfilePath) - 1);
  while (index >= 0 && FBfilePath[index] != '/')
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
  index = static_cast<int>(strlen(machine_define_path) - 1);
  while (index >= 0 && machine_define_path[index] != '/')
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
  ss >> thread_num;
  ss.clear();
  console->info("thread_num is {}", thread_num);

  return true;
}

bool parseMachineDefine(char const *machineDefinePath,
                        MachineManager *&machineManager) {

  auto console = spdlog::stdout_color_mt("parseMachineDefine");

  FILE *fp = fopen(machineDefinePath, "r"); // 非 Windows 平台使用 "r"
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

  assert(document.HasMember("clients"));
  Value clients;
  clients = document["clients"];
  assert(clients.IsArray());
  Value client;
  for (SizeType i = 0; i < clients.Size(); i++) {
    client = clients[i];
    assert(client.IsObject());
    char client_ip[64];
    int client_port;
    assert(client.HasMember("ip"));
    assert(client["ip"].IsString());
    ss << client["ip"].GetString();
    ss >> client_ip;
    ss.clear();

    assert(client.HasMember("port"));
    assert(client["port"].IsInt());
    ss << client["port"].GetInt();
    ss >> client_port;
    ss.clear();
    machineManager->addOnePhysicsMachine(client_ip, client_port);
  }

  //  const Value &clients_ip = document["clients_ip"];
  //  const Value &clients_port = document["clients_port"];
  //
  //  assert(clients_ip.IsArray());
  //  assert(clients_port.IsArray());
  //
  //  for (SizeType i = 0; i < clients_ip.Size(); i++) {
  //
  //    char client_ip[64];
  //    int client_port;
  //
  //    ss << clients_ip[i].GetString();
  //    ss >> client_ip;
  //    ss.clear();
  //
  //    ss << clients_port[i].GetInt();
  //    ss >> client_port;
  //    ss.clear();
  //
  //    machineManager->addOnePhysicsMachine(client_ip, client_port);
  //  }
}

int coflowSimMaster() {
  auto console = spdlog::stdout_color_mt("coflowSimMaster");

  int listenSockfd;
  // struct sockaddr_in listenAddr {};
  ThreadPool *pool = nullptr;
  CoflowBenchmarkTraceProducer producer(FBfilePath, "123");
  auto *scheduler1 = new Scheduler();
  auto *coflows = new vector<Coflow *>;
  auto *machineManager1 = new MachineManager();
  int coflow_num = 0;
  int flow_num = 0;

  epoll_event events[MAX_EVENT_NUMBER];
  int epollfd = epoll_create(MAX_EVENT_NUMBER);
  assert(epollfd != -1);
  // addfd(epollfd, listenSockfd, false);
  SocketManage::sEpollfd = epollfd;

  //解析coflow
  producer.prepareCoflows(coflows);

  // 统计coflow的数量和flow的数量
  coflow_num = coflows->size();
  for (int i = 0; i < coflow_num; i++) {
    flow_num += coflows->at(i)->getFlowsNum();
  }
  console->info("coflow_num: {}", coflow_num);
  console->info("flow_num: {}", flow_num);

  //  rapidjson::Document *coflows_json = new Document();
  //  coflows_json->SetObject();
  //  rapidjson::Document::AllocatorType& allocator =
  //  coflows_json->GetAllocator();
  //
  //  coflows_json->AddMember("id", rapidjson::Value(12), allocator);
  //  coflows_json->AddMember("name", rapidjson::Value("asd"), allocator);
  //
  //  coflows_json->AddMember("ar", Value(kArrayType), allocator);
  //  for(int i = 0; i < 5; i++){
  //    Value tmp_ob(kObjectType);
  //    tmp_ob.AddMember("id", rapidjson::Value(i), allocator);
  //    tmp_ob.AddMember("name", rapidjson::Value("asd"), allocator);
  //    coflows_json->operator[]("ar").PushBack(tmp_ob, allocator);
  //  }
  //  coflows_json->operator[]("ar").operator[](0).operator[]("id") = 7;
  //  StringBuffer buffer;
  //  Writer<StringBuffer> writer(buffer);
  //  coflows_json->Accept(writer);
  //  const string str = buffer.GetString();
  //  cout <<  str << endl;

  //  stringstream ssc;
  //  ssc << "{\"coflow\":[";
  //  ssc << coflows->at(0)->getCoflowJson();
  //  ssc << "]}";
  //  char strjson[1024];
  //  ssc >> strjson;
  //  cout << strjson <<endl;

  // machineManager处理
  machineManager1->setLogicMachineNum(150);
  parseMachineDefine(machine_define_path, machineManager1);
  machineManager1->startConn();

  scheduler1->setMachines(machineManager1);
  scheduler1->setCoflows(coflows);

  // 线程池
  try {
    pool = new ThreadPool(thread_num);
  } catch (...) {
    return 1;
  }
  pool->append(scheduler1);

  WebServer web_server("../web", 0);
  web_server.addHandler("/index", new IndexRequestHandler());
  web_server.addHandler("/coflowjson", new CoflowJsonHandler(coflows));
  web_server.setListen("127.0.0.1", 3000);
  web_server.start();

  //   while (true) {
  //
  //     int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
  //     if ((number < 0) && (errno != EINTR)) {
  //       console->error("epoll failure\n");
  //       break;
  //     }
  //     for (int i = 0; i < number; i++) {
  //       int sockfd = events[i].data.fd;
  //       if (sockfd == listenSockfd) {
  //         struct sockaddr_in client_address {};
  //         socklen_t client_addrlength = sizeof(client_address);
  //         int connfd = accept(listenSockfd, (struct sockaddr
  //         *)&client_address,
  //                             &client_addrlength);
  //         if (connfd < 0) {
  //           console->error("errno is: {}\n", errno);
  //           continue;
  //         }
  //         console->info("accept connfd: {}", connfd);
  //         //        machineManager1->addOnePhysicsMachine(connfd, connfd,
  //         //        client_address);
  //       } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
  //         //        machineManager1->removeOnePhysicsMachine(sockfd);
  //       } else if (events[i].events & EPOLLIN) {
  //         // machineManager1->getPhyMachineByMachineID(sockfd)->recvMsg();
  //         // pool->append(machineManager1->getPhyMachineByMachineID(sockfd));
  //       } else if (events[i].events & EPOLLOUT) {
  //         // machineManager1->getPhyMachineByMachineID(sockfd)->sendMsg();
  //       } else {
  //       }
  //     }
  //   }
}

int main(int argc, char const *argv[]) {
  parseConfig(argv[1]);
  coflowSimMaster();
  return 0;
}