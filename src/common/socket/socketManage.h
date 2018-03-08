//
// Created by he on 1/23/18.
//

#ifndef MASTER_SOCKETMANAGE_H
#define MASTER_SOCKETMANAGE_H

#include "../lib/locker.h"
#include "../lib/threadclass.h"
#include <netinet/in.h>
#define MSG_LEN 100

class SocketManage : public ThreadClass {
public:
  // 共用的epoll文件描述符
  static int sEpollfd;

  static const size_t MAX_RECV_BUFFER_SIZE = 2048;
  static const size_t MAX_SEND_BUFFER_SIZE = 1024;

  void initSocket(int sockfd, const sockaddr_in &addr);

  void closeConn(bool real_close = true);

  void run() override;

  bool setSendMsg(char *msg, int msgSize);

  bool getRecvBuf(char *&buf, int &buflen);
    int getM_recvIdx() const;
    void setM_recvIdx(int m_recvIdx);
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
