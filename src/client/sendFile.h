//
// Created by he on 3/9/18.
//

#ifndef COFLOWSIM_SENDFILE_H
#define COFLOWSIM_SENDFILE_H

#include <lib/threadclass.h>
#include <socket/socketManage.h>
#include <strings.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <arpa/inet.h>
#include <zconf.h>
using namespace std;

#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
class SendFile : public SocketManage {
public:
    SendFile(char* ins){
        reflag = false;
        int i;
        for(i = 0; i < strlen(ins); i++){
            this->ins[i] = ins[i];
        }
        this->ins[i++] = '\0';
    }
    void run(){
// 输入文件名 并放到缓冲区buffer中等待发送
        char file_name[FILE_NAME_MAX_SIZE];
        char file_save_name[FILE_NAME_MAX_SIZE];
//        bzero(file_name, FILE_NAME_MAX_SIZE+1);
//        printf("Please Input File Name On Server:\t");
//        gets( file_name);
//        printf("Please Input File Name to save On Client:\t");
//        gets(file_save_name);
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

        struct sockaddr_in connAddr;
        int connSockfd;
        memset(&connAddr, 0, sizeof(struct sockaddr_in));
        connAddr.sin_family = AF_INET;
        connAddr.sin_addr.s_addr = inet_addr(socketip);
        connAddr.sin_port = htons(socketport);
        if ((connSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0) {
            printf("Failed to create socket");
        }

        if (connect(connSockfd, (struct sockaddr*) &connAddr, sizeof(connAddr))<0) {
            printf("Failed to connect with server");
        }
        int error = 0;
        socklen_t len = sizeof(error);
        getsockopt(connSockfd, SOL_SOCKET, SO_ERROR, &error, &len);
        int reuse = 1;
        setsockopt(connSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));


        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        strncpy(buffer, file_name, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));
        if(send(connSockfd, buffer, BUFFER_SIZE, 0) < 0){
            perror("Send File Name Failed:");
            exit(1);
        }

        while (flowSizeMB-- > 0){
            memset(buffer,'a',sizeof(buffer));
            if(send(connSockfd, buffer, BUFFER_SIZE, 0) < 0){
                perror("Send File Failed:");
                exit(1);
            }
        }
        int nowTime = time(0);
        endTime = nowTime;
        cout << file_name << "发送完成，" << "时间：" << nowTime << endl;
        reflag = true;
        shutdown(connSockfd, SHUT_RDWR);

    }
    bool reflag;
    int coflowID;
    int flowID;
    int endTime;
private:
    char ins[FILE_NAME_MAX_SIZE];
};
#endif //COFLOWSIM_SENDFILE_H
