//
// Created by he on 4/13/18.
//
#include "TrafficControlManager.h"
#include<random>
#include <netinet/in.h>
#include <zconf.h>
#define COMMAND_MAX_LEN 256

const int start_port = 1001;
const char root_class_id[10] = "1:1";
const char root_id[10] = "1:0";

TrafficControlManager::TrafficControlManager(char *net_card_name, double bandwidth_MBs) {
  auto name_len = static_cast<int>(strlen(net_card_name));
  int i = 0;
  while (i < name_len) {
    this->net_card_name[i] = net_card_name[i];
    i++;
  }
  this->remain_bandwidth_MBs = bandwidth_MBs;
  this->initTC();
  class_max_num = (int) bandwidth_MBs;
}

void TrafficControlManager::initTC() {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd, "tc qdisc add dev %s root handle %s htb default 1", this->net_card_name, root_class_id);
  this->execShellCommmand(cmd);
  sprintf(cmd,
          "tc class add dev %s parent 1: classid %s htb rate %gMbit",
          net_card_name,
          root_class_id,
          remain_bandwidth_MBs);
  this->execShellCommmand(cmd);
}

int TrafficControlManager::getPortByBandwidth(int bandwidth_MBs) {
  get_port_locker.lock();
  // 在start_port 至 start_port+class_max_num之间随机获取一个可用的port
  default_random_engine e;
  uniform_int_distribution<int> rand_num(start_port, start_port + class_max_num);
  int port = rand_num(e);
  int rand_count = class_max_num/10;  // 表示随机的次数，超过次数则扩大class_max_num
  while (!this->isUnusedPort(port) && rand_count > 0) {
    port = rand_num(e);
    rand_count--;
  }

  // 随机次数超过class_max_num / 10，扩大class_max_num为原来的2倍
  if (rand_count==0) {
    class_max_num = 2*class_max_num;
    if (class_max_num + start_port >= PORT_MAX_NUM) {
      class_max_num = PORT_MAX_NUM - start_port - 1;
    }
    uniform_int_distribution<int> rand_num1(start_port, start_port + class_max_num);
    while (!this->isUnusedPort(port)) {
      port = rand_num1(e);
    }
  }
  this->setTcpPortBandwidth(port, bandwidth_MBs);
  get_port_locker.unlock();
  return port;
}

bool TrafficControlManager::setTcpPortBandwidth(int tcp_port, double bandwidth) {
  this->addTcClass(tcp_port, bandwidth);
  this->deleteTcFilter(tcp_port);
  this->addTcFilter(tcp_port, tcp_port);
  port_bandwitdth[tcp_port] = bandwidth;
  return true;
}

bool TrafficControlManager::isUnusedPort(int port) {
  int sock;
  struct sockaddr_in client{};

  if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    printf("Failed to create socket");
  }
  client.sin_family = AF_INET;
  client.sin_port = htons(static_cast<uint16_t>(port));
  client.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sock, (struct sockaddr *) &client, sizeof(struct sockaddr)) == -1) {
    return false;
  }
  close(sock);
  return true;
}

bool TrafficControlManager::execShellCommmand(char *command) {
  FILE *fstream = nullptr;
  char buff[1024];
  memset(buff, 0, sizeof(buff));

  if (nullptr == (fstream = popen(command, "r"))) {
    fprintf(stderr, "execute command failed: %s", strerror(errno));
    return false;
  }

  while (nullptr != fgets(buff, sizeof(buff), fstream)) {
    printf("%s", buff);
  }
  pclose(fstream);
  return true;
}

bool TrafficControlManager::addTcClass(int class_id, double bandwidth) {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd,
          "tc class replace dev %s parent %s classid 1:%d htb rate %gMbit",
          net_card_name,
          root_class_id,
          class_id,
          bandwidth);
  return this->execShellCommmand(cmd);
}
bool TrafficControlManager::changeTcClass(int class_id, double bandwidth) {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd,
          "tc change replace dev %s parent %s classid 1:%d htb rate %gMbit",
          net_card_name,
          root_class_id,
          class_id,
          bandwidth);
  return this->execShellCommmand(cmd);
}
bool TrafficControlManager::addTcFilter(int tcp_port, int flow_classid) {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd,
          "tc filter add dev %s parent %s prio %d protocol ip u32  match tcp src %d FFFF classid 1:%d",
          net_card_name,
          root_id,
          tcp_port,
          tcp_port,
          flow_classid);
  return this->execShellCommmand(cmd);
}
bool TrafficControlManager::changeTcFilter(int tcp_port, int flow_classid) {
  if (!deleteTcFilter(tcp_port)) {
    return false;
  }
  return addTcFilter(tcp_port, flow_classid);
}
bool TrafficControlManager::deleteTcFilter(int tcp_port) {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd,
          "tc filter delete dev %s parent %s prio %d u32",
          net_card_name,
          root_id,
          tcp_port);
  return this->execShellCommmand(cmd);
}


