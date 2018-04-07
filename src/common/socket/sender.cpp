//
// Created by he on 1/22/18.
//

#include "sender.h"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/syslog.h>
#include <zconf.h>

Sender::Sender(char *ip, int port) {
  struct sockaddr_in serv_addr {};
  if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    syslog(LOG_ERR, "Failed to create socket!");
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port = htons(port);

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    syslog(LOG_ERR, "Failed to connect: ", ip, ":", port);
  }
}

Sender::~Sender() {
  if (this->closeConnect() != 0) {
    shutdown(sock, SHUT_WR);
  }
}

char *Sender::sendStr(char *str) {
  if (send(sock, str, strlen(str), 0) != strlen(str)) {
    syslog(LOG_ERR, "Failed to send string");
  }
  return nullptr;
}

int Sender::closeConnect() { return close(sock); }
