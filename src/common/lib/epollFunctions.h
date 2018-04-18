//
// Created by he on 1/23/18.
//

#ifndef MASTER_EPOLLFUNCTIONS_H
#define MASTER_EPOLLFUNCTIONS_H

#include <fcntl.h>
#include <sys/epoll.h>
#include <zconf.h>

int setnonblocking(int fd);
void addfd(int epollfd, int fd, bool one_shot, bool non_block = true);
void removefd(int epollfd, int fd);
void modfd(int epollfd, int fd, int ev);
#endif // MASTER_EPOLLFUNCTIONS_H
