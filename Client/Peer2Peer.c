#include "Peer2Peer.h"

void initPeer2Peer(Peer2Peer* peer, int _port) {
    peer->udp_socket = create_UDPSocket(_port);
	peer->port = _port;
    peer->mthread = (MThread*)malloc(sizeof(MThread));
	peer->running = true;

    peer->mthread->run = runPeer2Peer;
    peer->mthread->arg = peer;
	mt_start(peer->mthread);
}

void runPeer2Peer(void *arg) {
	Peer2Peer* peer = (Peer2Peer*)arg;

	while(peer->running)
	{
		peer->buff = (char*)calloc(100, sizeof(char));

		if(recv_msg_udp(peer->udp_socket, peer->buff, 100) >= 0) {
			printf("Recieve message: %s\n", peer->buff);
		}

		free(peer->buff);
	}
}

void sendTo_udp(Peer2Peer* peer, char* msg, char* ip, int port) {
	if(peer == NULL || msg == NULL || ip == NULL) return;
	printf("send to ip: %s\n", ip);
	printf("send to port: %d\n", port);
		
	int num;
	num = sendTo(peer->udp_socket, msg, ip, port);
	printf("size of message sent: %ld\n", num);
	if(num < 0) {
		printf("Message could not be sent\n");
	}

}

// void Peer2Peer::close()
// {
// 	this->~MThread();
// 	socket->cclose();
// 	this->waitForThread();
// 	free(buff);
// }

// Peer2Peer::~Peer2Peer() {
// 	close();
// }

