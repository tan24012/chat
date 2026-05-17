#include "Server.h"

void initServer(Server* serv) {
	serv->loginAndSign = (LoginAndSignUp*)malloc(sizeof(LoginAndSignUp));
	initLoginAndSignUp(serv->loginAndSign);
	
	serv->mthread = (MThread*)malloc(sizeof(MThread));
    serv->listen_sock = NULL;
    serv->status = false;

    serv->mthread->run = runServer;
    serv->mthread->arg = serv;

    mt_start(serv->mthread);
}

void runServer(void* arg) {
    Server* serv = (Server*)arg;

    serv->status = true;
	serv->listen_sock = tcp_server_create(SERVER_PORT);
	sleep(2);
	printf("Server is listening on port %d\n", SERVER_PORT);

	while (serv->status == true)
	{

		TCPSocket* temp_sock = listenAndAccept(serv->listen_sock);

		if (!temp_sock)
			continue;
		else
		{
            printf("server accept client\n");
			addPeer(serv->loginAndSign, temp_sock);
		}
	}
	printf("Server stopped running\n");
}

void closeServer(Server* serv) {
    if (serv == NULL) {
        return;
    }

    serv->status = false;

    if (serv->listen_sock != NULL) {
        shutdown(serv->listen_sock->sockFd, SHUT_RDWR);
        close(serv->listen_sock->sockFd);
    }

    if (serv->mthread != NULL) {
        mt_wait(serv->mthread);
    }

    closeLoginAndSignUp(serv->loginAndSign);
    free(serv->loginAndSign);
    serv->loginAndSign = NULL;

    free(serv->listen_sock);
    serv->listen_sock = NULL;

    free(serv->mthread);
    serv->mthread = NULL;
}
