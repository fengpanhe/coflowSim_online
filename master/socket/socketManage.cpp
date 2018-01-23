//
// Created by he on 1/23/18.
//

#include <zconf.h>
#include <sys/epoll.h>
#include <bits/fcntl-linux.h>
#include <fcntl.h>
#include <cerrno>
#include <sys/uio.h>
#include <cstring>
#include "socketManage.h"


int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

void addfd( int epollfd, int fd, bool one_shot )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if( one_shot )
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    setnonblocking( fd );
}

void removefd( int epollfd, int fd )
{
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
    close( fd );
}

void modfd( int epollfd, int fd, int ev )
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
}


void SocketManage::initSocket(int sockfd, const sockaddr_in &addr) {
    mSockfd = sockfd;
    m_address = addr;
    int error = 0;
    socklen_t len = sizeof(error);
    getsockopt(mSockfd, SOL_SOCKET, SO_ERROR, &error, &len);
    int reuse = 1;
    setsockopt(mSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    addfd(this->sEpollfd, sockfd, true);
    init();
}

void SocketManage::init() {
    mRecvIdx = 0;
    mSendBufSize = 0;
    memset( mRecvBuf, '\0', MAX_RECV_BUFFER_SIZE );
    memset( mSendBuf, '\0', MAX_SEND_BUFFER_SIZE );
}

void SocketManage::closeConn(bool real_close) {
    if(real_close && (mSockfd != -1)){
        removefd(sEpollfd, mSockfd);
        mSockfd = -1;
    }
}

void SocketManage::run() {
    epoll_event event;
    event.data.fd = mSockfd;
    if (event.events & EPOLLIN){
        this->receiveMessage();
    }
    else if(event.events & EPOLLOUT){
        this->sendMessage();
    }
}

bool SocketManage::receiveMessage() {
    if(mRecvIdx >= MAX_RECV_BUFFER_SIZE){
        return false;
    }
    int bytes_read = 0;
    while (true){
        bytes_read = static_cast<int>(recv(mSockfd, mRecvBuf + mRecvIdx,
                                           static_cast<size_t>(MAX_RECV_BUFFER_SIZE - mRecvIdx), 0));
        if(bytes_read == -1){
            if (errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }
            return false;
        }
        else if(bytes_read == 0){
            return false;
        }
        mRecvIdx += bytes_read;
    }
    return true;
}

bool SocketManage::sendMessage() {

    if(mSendBufSize == 0){
        modfd(sEpollfd, mSockfd, EPOLLIN);
        init();
        return true;
    }

    int temp = 0;
    temp = static_cast<int>(send(mSockfd, mSendBuf, static_cast<size_t>(mSendBufSize), 0));

    if(temp == mSendBufSize){
        modfd(sEpollfd, mSockfd, EPOLLIN);
        init();
        return true;
    }

    if(temp <= -1){
        if( errno == EAGAIN ) {
            modfd( sEpollfd, mSockfd, EPOLLOUT );
            return true;
        }
        return false;
    }

    return false;
}

bool SocketManage::setSendMsg(char * msg, int msgSize) {
    if(mSendBufSize > 0){
        return false;
    }

    for (int i = 0; i < msgSize; ++i) {
        mSendBuf[i] = msg[i];
    }
    mSendBuf[msgSize] = '\0';
    mSendBufSize = msgSize;
    modfd( sEpollfd, mSockfd, EPOLLOUT );
    return true;
}


