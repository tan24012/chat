
#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include <netinet/in.h>
#include <inttypes.h>
#include <strings.h>
#include <stdio.h>

typedef struct {
    struct sockaddr_in  s_in;
	struct sockaddr_in from;
	unsigned int fsize;
	int socket_fd;
} UDPSocket;

UDPSocket* create_UDPSocket(int port);
int recv_msg_udp(UDPSocket* udpSocket, char* buffer, int length);
int sendTo(UDPSocket* udpSocket, char* msg, char* ip, int port);
void ccloseUdp(UDPSocket* udpSocket);
char* fromAddr();

#endif /* UDPSOCKET_H_ */
