//
// Created by he on 1/22/18.
//

#ifndef MASTER_SENDER_H
#define MASTER_SENDER_H

#include <sys/socket.h>

class Sender {
public:
  Sender(char *ip, int port);
  ~Sender();
  char *sendStr(char *str);
  int closeConnect();

private:
  int sock;
};

#endif // MASTER_SENDER_H
