//
// Created by he on 4/13/18.
//

#include "TrafficControlManager.h"
#include <netinet/in.h>
#include <random>
#include <zconf.h>
#define COMMAND_MAX_LEN 256

const int start_port = 1001;
const char root_class_id[10] = "1:1";
const char root_id[10] = "1:0";

TrafficControlManager::TrafficControlManager(char *net_card_name,
                                             double bandwidth_MBs) {
  auto name_len = static_cast<int>(strlen(net_card_name));
  int i = 0;
  while (i < name_len) {
    this->net_card_name[i] = net_card_name[i];
    i++;
  }
  this->net_card_name[i++] = '\0';
  this->remain_bandwidth_MBs = bandwidth_MBs;
  this->initTC();
}

void TrafficControlManager::initTC() {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd, "tc qdisc add dev %s root handle %s htb default 2",
          this->net_card_name, root_class_id);
  this->execShellCommmand(cmd);
  sprintf(cmd, "tc class replace dev %s parent 1: classid %s htb rate %lfMbit",
          net_card_name, root_class_id, remain_bandwidth_MBs);
  this->execShellCommmand(cmd);
  // 默认的分类,设置为1Mbit带宽
  this->addTcClass(2, 1);
}

bool TrafficControlManager::setIpPortBandwidth(int ip_port, double bandwidth) {
  if (bandwidth <= 0) {
    printf("WARNING: The bandwidth of ip port %d <= 0Mbit, set as default "
           "class.\n",
           ip_port);
    return false;
  }
  this->addTcClass(ip_port, bandwidth);
  this->deleteTcFilter(ip_port);
  this->addTcFilter(ip_port, ip_port);
  return true;
}

bool TrafficControlManager::execShellCommmand(char *command) {
//  printf("%s\n", command);
  FILE *fstream = nullptr;
  char buff[1024];
  memset(buff, 0, sizeof(buff));

  if (nullptr == (fstream = popen(command, "r"))) {
    fprintf(stderr, "execute command failed: %s", strerror(errno));
    return false;
  }
  bool flag = false;
  while (nullptr != fgets(buff, sizeof(buff), fstream)) {
    flag = true;
    printf("%s", buff);
  }
  pclose(fstream);
  if (flag) {
    printf("%s\n", command);
  }
  return true;
}

bool TrafficControlManager::addTcClass(int class_id, double bandwidth) {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd, "tc class replace dev %s parent %s classid 1:%x htb rate %lfMbit",
          net_card_name, root_class_id, class_id, bandwidth);
  return this->execShellCommmand(cmd);
}
bool TrafficControlManager::changeTcClass(int class_id, double bandwidth) {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd, "tc class replace dev %s parent %s classid 1:%x htb rate %lfMbit",
          net_card_name, root_class_id, class_id, bandwidth);
  return this->execShellCommmand(cmd);
}
bool TrafficControlManager::addTcFilter(int ip_port, int flow_classid) {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd,
          "tc filter replace dev %s parent %s prio %d protocol ip u32  match "
          "ip sport %d FFFF classid 1:%x",
          net_card_name, root_id, ip_port, ip_port, flow_classid);
  return this->execShellCommmand(cmd);
}
bool TrafficControlManager::changeTcFilter(int ip_port, int flow_classid) {
  if (!deleteTcFilter(ip_port)) {
    return false;
  }
  return addTcFilter(ip_port, flow_classid);
}
bool TrafficControlManager::deleteTcFilter(int ip_port) {
  char cmd[COMMAND_MAX_LEN];
  sprintf(cmd, "tc filter delete dev %s parent %s prio %d u32", net_card_name,
          root_id, ip_port);
  return this->execShellCommmand(cmd);
}
