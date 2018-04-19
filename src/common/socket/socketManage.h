//
// Created by he on 1/23/18.
//

#ifndef MASTER_SOCKETMANAGE_H
#define MASTER_SOCKETMANAGE_H

#include "../lib/locker.h"
#include "../lib/threadclass.h"
#include <iostream>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#define MSG_LEN 100

// auto socketManage_logger = spdlog::basic_logger_mt("socketManage_logger",
// "socketManage.log");

class SocketManage : public ThreadClass {
public:
  SocketManage();

  ~SocketManage() {
    // this->closeConn(); //加上这个会出错，未知
  }
  // 共用的epoll文件描述符
  static int sEpollfd;
  static const size_t MAX_RECV_BUFFER_SIZE = 2048;
  static const size_t MAX_SEND_BUFFER_SIZE = 1024;

  void initSocket(int sockfd, const sockaddr_in &addr);

  bool createConnect(char *sockip, int sockport);
  void closeConn(bool real_close = true);

  void run() override;

  bool setSendMsg(char *msg, int msgSize);
  bool getRecvBuf(char *&buf, int &buflen);

  int getM_recvIdx() const;
  void setM_recvIdx(int m_recvIdx);

  bool recvMsg();
  bool sendMsg();

private:
  sockaddr_in m_address;

  char m_recvBuf[MAX_RECV_BUFFER_SIZE];
  int m_recvIdx;
  locker m_recvBufLocker;

  char m_sendBuf[MAX_SEND_BUFFER_SIZE];
  int m_sendBufSize;
  locker m_sendBufLocker;

  void initRecvBuf();
  void initSendBuf();
  bool receiveMessage();
  bool sendMessage();

protected:
  int m_Sockfd;
};


#endif // MASTER_SOCKETMANAGE_H
