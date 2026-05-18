#include "Server.h"

void initServer(Server* serv) {
	serv->loginAndSign = (LoginAndSignUp*)malloc(sizeof(LoginAndSignUp));
	initLoginAndSignUp(serv->loginAndSign);
	
	serv->mthread = (MThread*)malloc(sizeof(MThread));
    serv->listen_sock = NULL;
    atomic_store(&serv->status, false);

    serv->mthread->run = runServer;
    serv->mthread->arg = serv;

    mt_start(serv->mthread);
}

void runServer(void* arg) {
    if(arg == NULL) return;

    Server* serv = (Server*)arg;

    atomic_store(&serv->status, true);
	serv->listen_sock = tcp_server_create(SERVER_PORT);
    if(serv->listen_sock == NULL) return;
	sleep(2);
	printf("Server is listening on port %d\n", SERVER_PORT);

	while (atomic_load(&serv->status) == true)
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

    atomic_store(&serv->status, false);

    if (serv->listen_sock != NULL) {
        shutdown(serv->listen_sock->sockFd, SHUT_RDWR);
        close(serv->listen_sock->sockFd);
    }

    if (serv->mthread != NULL) {
        mt_wait(serv->mthread);
    }

    closeLoginAndSignUp(serv->loginAndSign);

    if(serv->loginAndSign != NULL) {
        free(serv->loginAndSign);
        serv->loginAndSign = NULL;
    }
    
    if(serv->listen_sock != NULL) {
        free(serv->listen_sock);
        serv->listen_sock = NULL;
    }
    
    if(serv->mthread != NULL) {
        free(serv->mthread);
        serv->mthread = NULL;
    }
}
