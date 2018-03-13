//
// Created by he on 3/9/18.
//

#ifndef COFLOWSIM_SENDFILE_H
#define COFLOWSIM_SENDFILE_H

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <lib/threadclass.h>
#include <socket/socketManage.h>
#include <sstream>
#include <strings.h>
#include <zconf.h>
using namespace std;

#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

// auto sendFile_logger =
//     spdlog::basic_logger_mt("sendFile_logger", "sendFile_logger.log");
static std::shared_ptr<spdlog::logger> sendFile_logger;

auto sendFile_logger_ = spdlog::stdout_color_mt("sendFile_logger");
class SendFile : public SocketManage {
public:
  SendFile(char *ins, int inslen, SocketManage masterSockManger) {
    reflag = false;
    coflowID = -1;
    int i;
    for (i = 0; i < inslen; i++) {
      this->ins[i] = ins[i];
    }
    this->ins[i++] = '\0';
    this->masterSockManger = masterSockManger;
    try {
      spdlog::set_async_mode(8192);
      sendFile_logger = spdlog::rotating_logger_mt(
          "sendFile_logger", "sendFile_logger.log", 1024 * 1024 * 5, 3);
      spdlog::drop_all();
    } catch (const spdlog::spdlog_ex &ex) {
      std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
  }
  void run() {
    if (strlen(ins) <= 0) {
      cout << "ins Invalid" << endl;
      return;
    }
    // 输入文件名 并放到缓冲区buffer中等待发送
    char file_name[FILE_NAME_MAX_SIZE];
    char file_save_name[FILE_NAME_MAX_SIZE];

    char socketip[64] = "";
    int socketport = 0;
    double flowSizeMB = 0;
    double sendSpeedMbs = 0;

    stringstream ss;
    ss << this->ins;
    ss >> coflowID;
    ss >> flowID;
    ss >> socketip;
    ss >> socketport;
    ss >> file_name;
    ss >> flowSizeMB;
    ss >> sendSpeedMbs;
    ss.clear();

    // sendFile_logger->info("{}_{} 准备发送", coflowID, flowID);

    struct sockaddr_in connAddr;
    int connSockfd;
    memset(&connAddr, 0, sizeof(struct sockaddr_in));
    connAddr.sin_family = AF_INET;
    connAddr.sin_addr.s_addr = inet_addr(socketip);
    connAddr.sin_port = htons(socketport);
    if ((connSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      printf("Failed to create socket");
    }

    if (connect(connSockfd, (struct sockaddr *)&connAddr, sizeof(connAddr)) <
        0) {
      printf("Failed to connect with server");
    }
    int error = 0;
    socklen_t len = sizeof(error);
    getsockopt(connSockfd, SOL_SOCKET, SO_ERROR, &error, &len);
    int reuse = 1;
    setsockopt(connSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    //    strncpy(buffer, file_name, strlen(file_name) > BUFFER_SIZE ?
    //    BUFFER_SIZE : strlen(file_name)); if (send(connSockfd, buffer,
    //    BUFFER_SIZE, 0) < 0) {
    //      perror("Send File Name Failed:");
    //      exit(1);
    //    }
    long startTime = clock();
    // flowSizeMB = 1;
    int flowSizeKB = flowSizeMB * 1024;
    int sendenMB = 0;
    while (sendenMB++ >= flowSizeKB) {
      memset(buffer, 'a', sizeof(buffer));
      if (send(connSockfd, buffer, BUFFER_SIZE, 0) < 0) {
        perror("Send File Failed:");
        exit(1);
      }
    }
    reflag = true;
    removefd(SocketManage::sEpollfd, connSockfd);

    endTime = time(0);
    // sendFile_logger->info("{}_{} 发送完成, 当前时间： {}", coflowID, flowID,
    //                       endTime);
    sendFile_logger_->info("{}_{} Send Complete, S: {} MB, T: {}ms", coflowID,
                           flowID, flowSizeMB, clock() - startTime);
    char tmpstr[100] = "a";
    memset(tmpstr, '\0', strlen(tmpstr));
    tmpstr[0] = '(';
    ss << coflowID;
    ss >> tmpstr + strlen(tmpstr);
    ss.clear();

    tmpstr[strlen(tmpstr)] = ' ';

    ss << flowID;
    ss >> tmpstr + strlen(tmpstr);
    ss.clear();

    tmpstr[strlen(tmpstr)] = ' ';

    ss << endTime;
    ss >> tmpstr + strlen(tmpstr);
    ss.clear();

    tmpstr[strlen(tmpstr)] = ')';
    //    cout << tmpstr << endl;
    while (!masterSockManger.setSendMsg(tmpstr, strlen(tmpstr)))
      ;
    masterSockManger.sendMsg();
  }
  bool reflag;
  int coflowID;
  int flowID;
  long endTime;

private:
  SocketManage masterSockManger;
  char ins[FILE_NAME_MAX_SIZE];
};
#endif // COFLOWSIM_SENDFILE_H
