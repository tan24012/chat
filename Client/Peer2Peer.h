
#ifndef Peer2Peer_H_
#define Peer2Peer_H_

#include <stdbool.h>
#include "MThread.h"
#include "UDPSocket.h"

typedef struct {
    UDPSocket* udp_socket;
    MThread* mthread;
	int port;
	char* buff;
	bool running;
} Peer2Peer;

void initPeer2Peer(Peer2Peer* peer, int port);
void runPeer2Peer(void *arg);
void sendTo_udp(Peer2Peer* peer, char* msg, char* ip, int port);
void cclosePeer(Peer2Peer* peer);

#endif 
