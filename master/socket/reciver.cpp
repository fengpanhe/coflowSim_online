//
// Created by he on 1/22/18.
//

#include <fcntl.h>
#include <zconf.h>
#include <cerrno>
#include "reciver.h"

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


void Reciver::run() {

}

void Reciver::init(int sockfd, const sockaddr_in &addr) {
    m_sockfd = sockfd;
    m_address = addr;
    int error = 0;
    socklen_t len = sizeof(error);
    getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
    int reuse = 1;
    setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    addfd(this->m_epollfd, sockfd, true);
}

void Reciver::closeConn(bool real_close) {
    if(real_close && (m_sockfd != -1)){
        removefd(m_epollfd, m_sockfd);
        m_sockfd = -1;
    }
}

bool Reciver::read() {
    if(m_read_idx >= READ_BUFFER_SIZE){
        return false;
    }
    int bytes_read = 0;
    while (true){
        bytes_read = static_cast<int>(recv(m_sockfd, m_read_buf + m_read_idx,
                                           static_cast<size_t>(READ_BUFFER_SIZE - m_read_idx), 0));
        if(bytes_read == -1){
            if (errno == EAGAIN || errno == EWOULDBLOCK){
                break;
            }
            return false;
        }
        else if(bytes_read == 0){
            return false;
        }
        m_read_idx += bytes_read;
    }
    return true;
}

bool Reciver::write() {
    return false;
}
