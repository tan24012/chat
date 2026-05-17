#include "MultipleTCPSocketsListener.h"

TCPSocket* listenToSocket(MultipleTCPSocketsListener* listener, int count, int timeout) {
    struct timeval tv = {timeout, 0};

	fd_set fdset; 
	FD_ZERO(&fdset); 
	int maxfd=0;
    int returned;

    // đưa các fd vào tập 
	for (int i = 0; i < count; i++) {
		if (maxfd < listener->sockets[i]->sockFd) {
			maxfd = listener->sockets[i]->sockFd;
		}
		FD_SET(listener->sockets[i]->sockFd, &fdset);
	}

	if (timeout > 0) {
		returned = select(maxfd+1, &fdset, NULL, NULL, &tv);
	}
	else {
		returned = select(maxfd+1, &fdset, NULL, NULL, NULL);
	}

	if (returned)
	{
		for (int i = 0; i < count; i++)
		{
			TCPSocket* tmpSocket = listener->sockets[i];
			if (FD_ISSET(tmpSocket->sockFd, &fdset))
			{	
				return tmpSocket;
			}
		}
	}

	return NULL;
}