//
// Created by he on 3/9/18.
//

#ifndef COFLOWSIM_RECEFILE_H
#define COFLOWSIM_RECEFILE_H
#include <socket/socketManage.h>
#include <cstring>
#include <cstdio>
#include <zconf.h>
#include <cstdlib>
#include <fstream>
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
class ReceFile : public SocketManage {
public:
    void run()
    {
// recv函数接收数据到缓冲区buffer中
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        if (recv(m_Sockfd, buffer, BUFFER_SIZE, 0)<0) {
            perror("Server Recieve Data Failed:");
        }
        // 然后从buffer(缓冲区)拷贝到file_name中
        char file_name[FILE_NAME_MAX_SIZE+1];
        bzero(file_name, FILE_NAME_MAX_SIZE+1);
        strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));
        printf("%s\n", file_name);

        // 打开文件，准备写入
        FILE* fp = fopen(file_name, "w");
//        ofstream fp;
//        fp.open(file_name);
        if (NULL==fp) {
            printf("File:\t%s Can Not Open To Write\n", file_name);
        }

        // 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止
        bzero(buffer, BUFFER_SIZE);
        int length = 0;
        int allCount = 0;
        while ((length = (int) recv(m_Sockfd, buffer, BUFFER_SIZE, 0))>0) {
            if (fwrite(buffer, sizeof(char), length, fp)<length) {
                printf("File:\t%s Write Failed\n", file_name);
                break;
            }
//            fp << buffer;
            allCount += length;
            bzero(buffer, BUFFER_SIZE);
        }
//        fp.close();
        fclose(fp);
// 关闭连接
//        shutdown(m_Sockfd, SHUT_RD);
        this->closeConn();
        close(m_Sockfd);
    }
};
#endif //COFLOWSIM_RECEFILE_H
