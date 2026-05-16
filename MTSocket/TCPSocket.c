#include "TCPSocket.h"
#include "Protocol.h"


TCPSocket* tcp_server_create(int port) {
    TCPSocket* sock = (TCPSocket*)malloc(sizeof(TCPSocket));
    if(sock == NULL) {
        return NULL;
    }

	sock->sockFd = socket (AF_INET, SOCK_STREAM, 0);
    if(sock->sockFd < 0) {
        perror("socket");
        free(sock);
        return NULL;
    }
	memset(&sock->serverAddr, 0, sizeof(sock->serverAddr)); 

	//sets the sin address
	sock->serverAddr.sin_family = (short)AF_INET;
	sock->serverAddr.sin_port = htons((u_short)port);
    if (inet_pton(AF_INET, SERVER_IP, &sock->serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock->sockFd);
        free(sock);
        return NULL;
    }

    int opt = 1;
    setsockopt(sock->sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//bind the socket on the specified address
	if (bind(sock->sockFd,
         (struct sockaddr *)&sock->serverAddr,
         sizeof(sock->serverAddr)) < 0)
	{
		perror("bind");
        close(sock->sockFd);
        free(sock);
        return NULL;
	}

    printf("Bind succesfully\n");

    return sock;
}

TCPSocket* tcp_clent_create(char* peerIP, int port) {
    TCPSocket* sock = (TCPSocket*)malloc(sizeof(TCPSocket));
    if (sock == NULL) {
        return NULL;
    }

    if((sock->sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        free(sock);
        return NULL;
    }

    sock->peerAddr.sin_family = AF_INET;
    sock->peerAddr.sin_port = htons((u_short)port);
    if(inet_pton(AF_INET, peerIP, &sock->peerAddr.sin_addr) <= 0) {
        perror("Invalid peer IP");
        close(sock->sockFd);
        free(sock);
        return NULL;
    }

    if(connect(sock->sockFd, (struct sockaddr*)&sock->peerAddr, sizeof(sock->peerAddr)) < 0) {
        perror("Error establishing communications");
        close(sock->sockFd);
        free(sock);
        return NULL;
    }

    return sock;
}

TCPSocket* listenAndAccept(TCPSocket* s){
	int rc = listen(s->sockFd, 2);
	if (rc<0){
		return NULL;
	}

    socklen_t len = sizeof(s->peerAddr);
    memset(&s->peerAddr, 0, sizeof(s->peerAddr)); 

	int connect_sock_fd = accept(s->sockFd, (struct sockaddr *)&s->peerAddr, &len);
	if (connect_sock_fd < 0) {
        return NULL;
    }
/*
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,
            &s->peerAddr.sin_addr,
            ip,
            sizeof(ip));

    printf("Client: %s:%d\n",
       ip,
       ntohs(s->peerAddr.sin_port));
*/
	TCPSocket* sockRt = (TCPSocket*)malloc(sizeof(TCPSocket));
    sockRt->sockFd = connect_sock_fd;
    sockRt->peerAddr = s->peerAddr;
    sockRt->serverAddr = s->serverAddr;

    return sockRt;
}

int send_msg(int sockFd, const char* msg, int len){
	return write(sockFd, msg, len);
}

int recv_msg(int sockFd, char* buffer, int length){
	return read(sockFd, buffer, length);
}

char* destIpAndPort(TCPSocket* s){
	char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &s->peerAddr.sin_addr, ip, sizeof(ip));
    int port = ntohs(s->peerAddr.sin_port);

    // đủ cho "255.255.255.255:65535\0"
    char* result = malloc(22);
    if (result == NULL)
        return NULL;
    sprintf(result, "%s:%d", ip, port);

    return result;
}

void cclose(TCPSocket* s) {
	printf("closing socket\n");
	shutdown(s->sockFd,SHUT_RDWR);
	close(s->sockFd);
}