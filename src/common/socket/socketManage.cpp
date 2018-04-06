//
// Created by he on 1/23/18.
//

//#include <zconf.h>
#include "socketManage.h"
#include "../../../thirdparty/spdlog/spdlog.h"
#include "../lib/epollFunctions.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>

#include <iostream>
using namespace std;

int SocketManage::sEpollfd = -1;

const auto socketManage_logger_console =
    spdlog::stdout_color_mt("socketManage_logger");

static std::shared_ptr<spdlog::logger> socketManage_logger_file;

SocketManage::SocketManage() {
  m_Sockfd = -1;
  try {
    spdlog::set_async_mode(8192);
    socketManage_logger_file = spdlog::rotating_logger_mt(
        "socketManage_logger", "socketManage_logger.log", 1024 * 1024 * 5, 3);
    spdlog::drop_all();
  } catch (const spdlog::spdlog_ex &ex) {
    std::cout << "Log initialization failed: " << ex.what() << std::endl;
  }
}

void SocketManage::initSocket(int sockfd, const sockaddr_in &addr) {
  m_Sockfd = sockfd;
  m_address = addr;
  int error = 0;
  socklen_t len = sizeof(error);
  getsockopt(m_Sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
  int reuse = 1;
  setsockopt(m_Sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  addfd(SocketManage::sEpollfd, sockfd, false);
  initRecvBuf();
  initSendBuf();
}

bool SocketManage::createConnect(char *sockip, int sockport) {
  if (m_Sockfd != -1) {
    this->closeConn();
  }
  struct sockaddr_in connAddr;
  int connSockfd;
  memset(&connAddr, 0, sizeof(struct sockaddr_in));
  connAddr.sin_family = AF_INET;
  connAddr.sin_addr.s_addr = inet_addr(sockip);
  connAddr.sin_port = htons(sockport);
  if ((connSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    socketManage_logger_console->error("Failed to create socket! \n");
  }

  if (connect(connSockfd, (struct sockaddr *)&connAddr, sizeof(connAddr)) < 0) {
    socketManage_logger_console->error("Failed to connect with server!\n");
  }
  int error = 0;
  socklen_t len = sizeof(error);
  getsockopt(connSockfd, SOL_SOCKET, SO_ERROR, &error, &len);
  int reuse = 1;
  setsockopt(connSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  socketManage_logger_console->info("Connected! sockfd is {}", connSockfd);
  this->initSocket(connSockfd, connAddr);
}

void SocketManage::closeConn(bool real_close) {
  if (real_close && (m_Sockfd != -1)) {
    removefd(SocketManage::sEpollfd, m_Sockfd);
    m_Sockfd = -1;
  }
}

void SocketManage::run() {
  m_sendBufLocker.lock();
  if (m_sendBufSize > 0) {
    printf("%d send msg 1 %s \n", m_Sockfd, m_sendBuf);
    this->sendMessage();
    //        printf("%d send msg 2\n", m_Sockfd);
    m_sendBufLocker.unlock();
    return;
  }
  m_sendBufLocker.unlock();

  m_recvBufLocker.lock();
  if (!this->receiveMessage()) {
    cout << "error:" << endl;
  }
  //  printf("recvmsg: \"%s\" \n", m_recvBuf);

  m_recvBufLocker.unlock();
}

bool SocketManage::receiveMessage() {
  if (m_recvIdx > MAX_RECV_BUFFER_SIZE) {
    printf("m_recvIdx(%d) >= MAX_RECV_BUFFER_SIZE(%d)\n", m_recvIdx,
           MAX_RECV_BUFFER_SIZE);
    return false;
  }
  int bytes_read = 0;
  while (true) {
    bytes_read = static_cast<int>(
        recv(m_Sockfd, m_recvBuf + m_recvIdx,
             static_cast<size_t>(MAX_RECV_BUFFER_SIZE - m_recvIdx), 0));
    if (bytes_read == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }
      printf("receiveMessage error: bytes_read==-1 \n");
      return false;
    } else if (bytes_read == 0) {
      printf("receiveMessage error: bytes_read==0 \n");
      return false;
    }
    m_recvIdx += bytes_read;
    if (m_recvIdx >= MSG_LEN) {
      break;
    }
  }
  //  printf("recvmsg: \"%s\" \n", m_recvBuf);
  socketManage_logger_file->info("recvmsg: \"{}\" ", m_recvBuf);
  return true;
}

bool SocketManage::sendMessage() {

  if (m_sendBufSize == 0) {
    modfd(SocketManage::sEpollfd, m_Sockfd, EPOLLIN);
    initSendBuf();
    return true;
  }

  int temp = 0;
  temp = static_cast<int>(
      send(m_Sockfd, m_sendBuf, static_cast<size_t>(m_sendBufSize), 0));

  if (temp == m_sendBufSize) {
    modfd(SocketManage::sEpollfd, m_Sockfd, EPOLLIN);
    //    printf("sendmsg: \"%s\" \n", m_sendBuf);
    socketManage_logger_file->info("sendmsg: \"{}\" ", m_sendBuf);
    initSendBuf();
    return true;
  }

  if (temp <= -1) {
    if (errno == EAGAIN) {
      modfd(SocketManage::sEpollfd, m_Sockfd, EPOLLOUT);
      return true;
    }
    return false;
  }

  return false;
}

bool SocketManage::setSendMsg(char *msg, int msgSize) {
  m_sendBufLocker.lock();
  if ((m_sendBufSize + msgSize) > MAX_SEND_BUFFER_SIZE) {
    m_sendBufLocker.unlock();
    return false;
  }
  for (int i = 0; i < msgSize; ++i) {
    m_sendBuf[m_sendBufSize++] = msg[i];
  }
  m_sendBuf[m_sendBufSize] = '\0';

  //  m_sendBufSize = (msgSize/MSG_LEN + 1)*MSG_LEN;

  modfd(SocketManage::sEpollfd, m_Sockfd, EPOLLOUT);
  m_sendBufLocker.unlock();
  return true;
}

void SocketManage::initRecvBuf() {
  m_recvIdx = 0;
  memset(m_recvBuf, '\0', MAX_RECV_BUFFER_SIZE);
}

void SocketManage::initSendBuf() {
  m_sendBufSize = 0;
  memset(m_sendBuf, '\0', MAX_SEND_BUFFER_SIZE);
}

bool SocketManage::getRecvBuf(char *&buf, int &buflen) {
  m_recvBufLocker.lock();
  int i = 0;
  while (i < m_recvIdx) {
    buf[buflen++] = m_recvBuf[i];
    i++;
  }
  buf[buflen] = '\0';
  initRecvBuf();
  m_recvBufLocker.unlock();
  return false;
}
int SocketManage::getM_recvIdx() const { return m_recvIdx; }
void SocketManage::setM_recvIdx(int m_recvIdx) {
  SocketManage::m_recvIdx = m_recvIdx;
}
bool SocketManage::recvMsg() {
  m_recvBufLocker.lock();
  if (!this->receiveMessage()) {
    cout << "error:recvMsg()" << endl;
  }
  //  printf("recvmsg: \"%s\" \n", m_recvBuf);
  m_recvBufLocker.unlock();
  return true;
}
bool SocketManage::sendMsg() {
  m_sendBufLocker.lock();
  if (m_sendBufSize > 0) {
    //    printf("%d send msg 1 %s \n", m_Sockfd, m_sendBuf);
    this->sendMessage();
    //        printf("%d send msg 2\n", m_Sockfd);
    m_sendBufLocker.unlock();
    return true;
  }
  m_sendBufLocker.unlock();
  return false;
}
