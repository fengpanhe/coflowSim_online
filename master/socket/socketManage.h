//
// Created by he on 1/23/18.
//

#ifndef MASTER_SOCKETMANAGE_H
#define MASTER_SOCKETMANAGE_H


#include <netinet/in.h>
#include "../lib/thread.h"

class SocketManage : public thread {
public:
    // 共用的epoll文件描述符
    static int sEpollfd;

    const size_t MAX_RECV_BUFFER_SIZE = 2048;
    const size_t MAX_SEND_BUFFER_SIZE = 1024;

    void initSocket(int sockfd, const sockaddr_in & addr);
    void closeConn(bool real_close = true);
    void run() override;
    bool receiveMessage();
    bool sendMessage();

    bool setSendMsg(char * msg, int msgSize);


private:
    int mSockfd;
    sockaddr_in m_address;
    char mRecvBuf[MAX_RECV_BUFFER_SIZE];
    int mRecvIdx;

    char mSendBuf[MAX_SEND_BUFFER_SIZE];
    int mSendBufSize;

    void init();
};


#endif //MASTER_SOCKETMANAGE_H
