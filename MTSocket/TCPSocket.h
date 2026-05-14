#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct {
    struct sockaddr_in serverAddr;
    struct sockaddr_in peerAddr;
    int sockFd;
}TCPSocket;

TCPSocket* tcp_server_create(int port);
TCPSocket* tcp_clent_create(char* peerIP, int port);
TCPSocket* listenAndAccept(TCPSocket* s);
int send_msg(int sockFd, const char* msg, int len);
int recv_msg(int sockFd, char* buffer, int len);
char* destIpAndPort(TCPSocket* s);  // Trả về chuỗi "IP:port" của peer socket

#endif