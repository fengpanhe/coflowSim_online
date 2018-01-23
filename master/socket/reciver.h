//
// Created by he on 1/22/18.
//

#ifndef MASTER_RECIVER_H
#define MASTER_RECIVER_H


#include <netinet/in.h>
#include <sys/epoll.h>
#include "../lib/thread.h"

class Reciver : public thread {
public:
    // 共用的epoll文件描述符
    static int m_epollfd;

    const int READ_BUFFER_SIZE = 2048;
    const int WRITE_BUFFER_SIZE = 1024;

    void init(int sockfd, const sockaddr_in & addr);
    void closeConn(bool real_close = true);
    void run() override;
    bool read();
    bool write();

private:
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[ READ_BUFFER_SIZE ];
    int m_read_idx;

    char m_write_buf[ WRITE_BUFFER_SIZE ];
    int m_write_idx;

};


#endif //MASTER_RECIVER_H
