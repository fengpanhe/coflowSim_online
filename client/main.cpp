#include <iostream>
#include <netinet/in.h>
#include <cstring>
#include <zconf.h>
#include <sstream>
#include <arpa/inet.h>

using namespace std;
#define BROADCAST_LISTEN_PORT 4001
#define CONN_PORT 4002
int test() {
    int bcListenSockfd, connSockfd;
    struct sockaddr_in bclAddr, connAddr;


    if ((bcListenSockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("socket fail\n");
        return -1;
    }
    int set = 1;
    setsockopt(bcListenSockfd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));
    memset(&bclAddr, 0, sizeof(struct sockaddr_in));
    bclAddr.sin_family = AF_INET;
    bclAddr.sin_port = htons(BROADCAST_LISTEN_PORT);
    bclAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(bcListenSockfd, (struct sockaddr *) &bclAddr, sizeof(struct sockaddr)) ==
        -1) {
        printf("bind fail\n");
        return -1;
    }
    int recvbytes;
    char recvbuf[128];
    socklen_t addrLen = sizeof(struct sockaddr_in);
    if ((recvbytes = recvfrom(bcListenSockfd, recvbuf, 128, 0, (struct sockaddr *) &bclAddr, &addrLen)) != -1) {
        recvbuf[recvbytes] = '\0';
        printf("receive a broadCast messgse:%s\n", recvbuf);
    } else {
        printf("recvfrom fail\n");
    }
    close(bcListenSockfd);

    stringstream ss(recvbuf);
    char ip[20];
    int port;
    ss >> ip;
    ss >> port;
    printf("ip: %s, port: %d\n", ip, port);
    memset(&connAddr, 0, sizeof(struct sockaddr_in));
    connAddr.sin_family = AF_INET;
    connAddr.sin_addr.s_addr = inet_addr(ip);
    connAddr.sin_port = htons(port);
    if ((connSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("Failed to create socket");
    }

    if (connect(connSockfd, (struct sockaddr *) &connAddr, sizeof(connAddr)) < 0) {
        printf("Failed to connect with server");
    }
    printf("%s\n", "connected");
    while (true) {
        printf("%s\n", "recv");
        int bytes_read = 0;
        char buf[1024];
        printf("%s\n", "recv");
        while (true) {
            bytes_read = 0;
            bytes_read = static_cast<int>(recv(connSockfd, buf + bytes_read,
                                               static_cast<size_t>(100 - bytes_read), 0));
            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                printf("Failed");
                return false;
            } else if (bytes_read == 0) {
                printf("Failed");
                return false;
            }
            printf("buf: %s\n", buf);
        }
        printf(buf);
        printf("\n");
    }
}
int main() {
    std::cout << "Hello, World!" << std::endl;
    test();
    return 0;
}