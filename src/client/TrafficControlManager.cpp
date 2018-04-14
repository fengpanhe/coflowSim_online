//
// Created by he on 4/13/18.
//


#include "TrafficControlManager.h"

#define COMMAND_MAX_LEN 256

const int start_port = 10001;
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
  class_max_num = bandwidth_MBs;
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
  // TO 在startport至startport+class_max_num范围内随机一个端口号，一定次数随机不出，则class_max_num加一
  return 0;
}

bool TrafficControlManager::setTcpPortBandwidth(int tcp_port, double bandwidth) {
  this->addTcClass(tcp_port, bandwidth);
  this->deleteTcFilter(tcp_port);
  this->addTcFilter(tcp_port, tcp_port);
  port_bandwitdth[tcp_port] = bandwidth;
  return true;
}

bool TrafficControlManager::isUnusedPort(int port) {
  return false;
}

bool TrafficControlManager::execShellCommmand(char *command) {
  FILE *fstream = NULL;
  char buff[1024];
  memset(buff, 0, sizeof(buff));

  if (NULL==(fstream = popen(command, "r"))) {
    fprintf(stderr, "execute command failed: %s", strerror(errno));
    return false;
  }

  while (NULL!=fgets(buff, sizeof(buff), fstream)) {
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
  if(!deleteTcFilter(tcp_port)){
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


