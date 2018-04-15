//
// Created by he on 4/13/18.
// 流量控制，使用Linux 的 tc 命令，相关请见 man tc
//

#ifndef COFLOWSIM_TRAFFICCONTROLMANAGER_H
#define COFLOWSIM_TRAFFICCONTROLMANAGER_H

#include <map>
#include <list>
#include <string.h>
#include <lib/locker.h>
using namespace std;

#define NET_CARD_NAME_MAX_LEN 1024
#define PORT_MAX_NUM 65535

// tc的控制设计qdisc，class，filter
// 此类对流量的控制是仅用于限制 tcp 端口流量
// qdisc使用htb，
// class的id 与 tcp端口 相同
// filter的prio 与 tcp端口 相同
class TrafficControlManager {
public:
  TrafficControlManager(char *net_card_name, double bandwidth_MBs);

  // 获得一个限速为bandwidth的可用的端口
  int getPortByBandwidth(int bandwidth_MBs);
  // 设置tcp端口的带宽
  bool setTcpPortBandwidth(int tcp_port, double bandwidth);

private:

  // 判断端口是否可以使用占用，未被占用
  bool isUnusedPort(int port);
  // 执行Linux shell命令
  bool execShellCommmand(char *command);
  // 初始化tc队列
  void initTC();

  // 在tc队列根类下添加一个类，id为class_id，限制带宽为bandwidth
  bool addTcClass(int class_id, double bandwidth);
  // 在tc队列根类下更改一个类，id为class_id，限制带宽为bandwidth
  bool changeTcClass(int class_id, double bandwidth);

  // 添加一个tc过滤器，要限制参数tcp的端口，流向的类id；优先级与tcp端口同，便于管理
  bool addTcFilter(int tcp_port, int flow_classid);
  // 改变一个tc过滤器，要限制参数tcp的端口，流向的类id； 优先级与tcp端口同，便于管理
  bool changeTcFilter(int tcp_port, int flow_classid);
  // 删除优先级为 tcp_port 的所有的filter
  bool deleteTcFilter(int tcp_port);

  char net_card_name[NET_CARD_NAME_MAX_LEN];
  double remain_bandwidth_MBs;
  double port_bandwitdth[PORT_MAX_NUM];   // 端口的带宽，其中端口与classid同
  int class_max_num;
  locker get_port_locker;
//  map<>
};

#endif //COFLOWSIM_TRAFFICCONTROLMANAGER_H
