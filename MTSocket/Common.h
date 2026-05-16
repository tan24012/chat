#include "TCPSocket.h"

static int readCommand(TCPSocket* socktoread) {
	if (socktoread == NULL) {
		return;
	}
	int messagelength = 0, recived = 0;
	recived = recv_msg(socktoread->sockFd, (char*)&messagelength, 4);
	if(!(recived==4))
		return 0;
	return ntohl((uint32_t)messagelength);
}

static void writeCommand(TCPSocket* socktowrite, int command) {
	if (socktowrite == NULL) {
		return;
	}
	int msglen = htonl((uint32_t)command);
	send_msg(socktowrite->sockFd, (char*)&msglen, 4);
}

static void writeMsg(TCPSocket* socktowrite, char* msg) {
	if (socktowrite == NULL || msg == NULL) {
		return;
	}
	//send msg length
	int msglen = htonl((uint32_t) strlen(msg));
	send_msg(socktowrite->sockFd, (char*)&msglen,4);
	//send msg
	send_msg(socktowrite->sockFd, msg, strlen(msg));
}

static char* readMsg(TCPSocket* socktoread)
{
	int received = 0, messagelength = 0, len = 0;
	char* rcvmsg = NULL;

	if (socktoread == NULL) {
		return NULL;
	}

	received = recv_msg(socktoread->sockFd, (char*)&messagelength, 4);
	if (received != 4) {
		return NULL;
	}

	len = ntohl((uint32_t)messagelength);
	if (len <= 0 || len > 4096) {
		return NULL;
	}

	rcvmsg = malloc(len + 1);
	if (rcvmsg == NULL) {
		return NULL;
	}

	received = recv_msg(socktoread->sockFd, rcvmsg, len);
	if (received != len) {
		free(rcvmsg);
		return NULL;
	}

	rcvmsg[len] = '\0';
	return rcvmsg;
}