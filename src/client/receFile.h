//
// Created by he on 3/9/18.
//

#ifndef COFLOWSIM_RECEFILE_H
#define COFLOWSIM_RECEFILE_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <zconf.h>
#include <lib/ThreadPool.h>
#include <netinet/in.h>
using namespace std;

#define RECEFILE_BUFFER_SIZE 1024
#define RECEFILE_FILE_NAME_MAX_SIZE 512
class ReceFile : public ThreadClass {
public:
  int file_int = 0;
  static int rf_epollfd;
  void initSocket(int sockfd, const sockaddr_in &addr);
  void run() override;
  void closeConn(bool real_close = true);
private:
  int m_Sockfd;
//  sockaddr_in m_address;
};
#endif // COFLOWSIM_RECEFILE_H
