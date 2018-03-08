//
// Created by he on 1/23/18.
//

#include <zconf.h>
#include <sys/epoll.h>
#include <cerrno>
#include <cstring>
#include "socketManage.h"
#include "../lib/epollFunctions.h"

#include <iostream>
using namespace std;

int SocketManage::sEpollfd = -1;

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

void SocketManage::closeConn(bool real_close) {
    if(real_close && (m_Sockfd != -1)){
        removefd(SocketManage::sEpollfd, m_Sockfd);
        m_Sockfd = -1;
    }
}

void SocketManage::run() {
    m_sendBufLocker.lock();
    if(m_sendBufSize > 0){
//        printf("%d send msg 1 %s \n", m_Sockfd, m_sendBuf);
        this->sendMessage();
//        printf("%d send msg 2\n", m_Sockfd);
        m_sendBufLocker.unlock();
        return;
    }
    m_sendBufLocker.unlock();

    m_recvBufLocker.lock();
    if(!this->receiveMessage()){
        cout << "error:" << endl;
    }
    printf("recvmsg: \"%s\" \n", m_recvBuf);
//    initRecvBuf();
    m_recvBufLocker.unlock();
}

bool SocketManage::receiveMessage() {
    if(m_recvIdx >= MAX_RECV_BUFFER_SIZE){
        return false;
    }
    int bytes_read = 0;
    while (true){
        bytes_read = static_cast<int>(recv(m_Sockfd, m_recvBuf + m_recvIdx,
                                           static_cast<size_t>(MAX_RECV_BUFFER_SIZE - m_recvIdx), 0));
        if(bytes_read == -1){
            if (errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }
            return false;
        }
        else if(bytes_read == 0){
            return false;
        }
        m_recvIdx += bytes_read;
        if (m_recvIdx >= MSG_LEN){
            break;
        }
    }
    return true;
}

bool SocketManage::sendMessage() {

    if(m_sendBufSize == 0){
        modfd(SocketManage::sEpollfd, m_Sockfd, EPOLLIN);
        initSendBuf();
        return true;
    }

    int temp = 0;
    temp = static_cast<int>(send(m_Sockfd, m_sendBuf, static_cast<size_t>(m_sendBufSize), 0));

    if(temp == m_sendBufSize){
        modfd(SocketManage::sEpollfd, m_Sockfd, EPOLLIN);
        initSendBuf();
        return true;
    }

    if(temp <= -1){
        if( errno == EAGAIN ) {
            modfd(SocketManage::sEpollfd, m_Sockfd, EPOLLOUT );
            return true;
        }
        return false;
    }

    return false;
}

bool SocketManage::setSendMsg(char * msg, int msgSize) {
    m_sendBufLocker.lock();
    if ((m_sendBufSize + msgSize) > MAX_SEND_BUFFER_SIZE){
        m_sendBufLocker.unlock();
        return false;
    }
    for (int i = 0; i < msgSize; ++i) {
        m_sendBuf[m_sendBufSize++] = msg[i];
    }
    m_sendBuf[m_sendBufSize] = '\0';

    m_sendBufSize = (msgSize / MSG_LEN + 1) * MSG_LEN;

    modfd(SocketManage::sEpollfd, m_Sockfd, EPOLLOUT );
    m_sendBufLocker.unlock();
    return true;
}

void SocketManage::initRecvBuf() {
    m_recvIdx = 0;
    memset( m_recvBuf, '\0', MAX_RECV_BUFFER_SIZE );
}

void SocketManage::initSendBuf() {
    m_sendBufSize = 0;
    memset( m_sendBuf, '\0', MAX_SEND_BUFFER_SIZE );
}

bool SocketManage::getRecvBuf(char *&buf, int &buflen) {
    m_recvBufLocker.lock();
    int i = 0;
    while (i < m_recvIdx){
        buf[i] = m_recvBuf[i];
        i++;
    }
    buf[i] = '\0';
    buflen = m_recvIdx;
    initRecvBuf();

    m_recvBufLocker.unlock();
    return false;
}
int SocketManage::getM_recvIdx() const
{
    return m_recvIdx;
}
void SocketManage::setM_recvIdx(int m_recvIdx)
{
    SocketManage::m_recvIdx = m_recvIdx;
}


