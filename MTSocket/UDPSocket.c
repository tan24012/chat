

#include "UDPSocket.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

UDPSocket* create_UDPSocket(int port){
    UDPSocket* udpSocket = (UDPSocket*)malloc(sizeof(UDPSocket));

	udpSocket->socket_fd = socket (AF_INET, SOCK_DGRAM, 0);
	if (udpSocket->socket_fd < 0) {
		perror("socket");
		return;
	}
	memset(&udpSocket->s_in, 0, sizeof(udpSocket->s_in));

	udpSocket->s_in.sin_family = (short)AF_INET;
	udpSocket->s_in.sin_addr.s_addr = htonl(INADDR_ANY);   
	udpSocket->s_in.sin_port = htons((u_short)port);

	udpSocket->fsize = sizeof(udpSocket->from);

	if(bind(udpSocket->socket_fd, (struct sockaddr *)&udpSocket->s_in, sizeof(udpSocket->s_in)) < 0){
		perror ("Error naming channel");
	}
    return udpSocket;
}

int recv_msg_udp(UDPSocket* udpSocket, char* buffer, int length) {
	return recvfrom(udpSocket->socket_fd, buffer, length, 0, (struct sockaddr *)&udpSocket->from, &udpSocket->fsize);
}

int sendTo(UDPSocket* udpSocket, char* msg, char* ip, int port){
	struct sockaddr_in toAddr;
    memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family = AF_INET;
	toAddr.sin_addr.s_addr = inet_addr(ip);
	toAddr.sin_port = htons(port);
	return sendto(udpSocket->socket_fd, msg, strlen(msg), 0, (struct sockaddr *)&toAddr, sizeof(toAddr));
}

void ccloseUdp(UDPSocket* udpSocket) {
	printf("closing udp socket ..\n");
	shutdown(udpSocket->socket_fd, SHUT_RDWR);
	close(udpSocket->socket_fd);
}



