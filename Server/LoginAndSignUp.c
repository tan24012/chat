#include "LoginAndSignUp.h"

void initLoginAndSignUp(LoginAndSignUp* loginAndSign) {
    loginAndSign->mthread = (MThread*)malloc(sizeof(MThread));
    memset(&loginAndSign->peers, 0, sizeof(loginAndSign->peers)); 

	loginAndSign->dispatcher = (Dispatcher*)malloc(sizeof(Dispatcher));
	initDispatcher(loginAndSign->dispatcher);

    loginAndSign->peers_count = 0;
    loginAndSign->status = false;
	pthread_mutex_init(&loginAndSign->peers_mutex, NULL);
}

void addPeer(LoginAndSignUp* loginAndSign, TCPSocket* peer) {
	pthread_mutex_lock(&loginAndSign->peers_mutex);
    if (loginAndSign->peers_count < MAX_PEERS) {
        loginAndSign->peers[loginAndSign->peers_count] = peer;
        loginAndSign->peers_count++;
    } else {
        printf("Maximum number of peers reached. Cannot add more.\n");
    }
	pthread_mutex_unlock(&loginAndSign->peers_mutex)

    if(loginAndSign->status == false) {
        loginAndSign->status = true;
        loginAndSign->mthread->run = runLoginAndSignUp;
        loginAndSign->mthread->arg = (void*)loginAndSign;
		mt_start(loginAndSign->mthread);
    }
}

bool login(Dispatcher* dispatcher, char* username, char* password) {
	FILE* f = fopen("/home/hoangtan/duytan/chat/Server/users.txt", "r");
	if (f == NULL || username == NULL || password == NULL) {
		printf("Error opening users.txt in login\n");
		return false;
	}

	char user[100];
	char pass[100];	
	bool flag = false;
	bool userExist = false;

	while(fscanf(f, "%s %s", user, pass) == 2) {
		if(strcmp(username, user)==0 && strcmp(password, pass)==0) {
			flag = true;
			break;
		}
	}
	fclose(f);

	userExist = checkLogined(dispatcher, username);

	if(flag && userExist == false) {
		return true;
	}

	return false;
}

bool signup(Dispatcher* dispatcher, char* username, char* password)
{
	FILE* f = fopen("../Server/users.txt", "r");
	if (f == NULL) {
		printf("Error opening users.txt in sign up\n");
		return false;
	}

	char user[100];
	char pass[100];
	bool flag = false;

	while(fscanf(f, "%s %s", user, pass) == 2) {
		if(strcmp(username, user) == 0) {
			flag = true;	// trùng tên đăng ký
			break;
		}
	}
	if(flag) {
		fclose(f);
		return false;
	}

	f = fopen("/home/hoangtan/duytan/chat/Server/users.txt", "a");
	if (f == NULL) {
		printf("Error opening users.txt in sign up write\n");
		return false;
	}
	fprintf(f, "%s %s\n", username, password);
	fclose(f);

	return true;
}

void remove_peer(LoginAndSignUp* loginAndSign, TCPSocket* peer) {
	pthread_mutex_lock(&loginAndSign->peers_mutex);
    int index = -1;

    for (int i = 0; i < loginAndSign->peers_count; i++) {
        if (loginAndSign->peers[i] == peer) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return;
    }

    for (int i = index; i < loginAndSign->peers_count - 1; i++) {
        loginAndSign->peers[i] = loginAndSign->peers[i + 1];
    }

    loginAndSign->peers_count--;
    loginAndSign->peers[loginAndSign->peers_count] = NULL;
	pthread_mutex_unlock(&loginAndSign->peers_mutex)
}

void runLoginAndSignUp(void* arg) {
	printf("Run Login and Sign-up thread\n");
    LoginAndSignUp* loginAndSign = (LoginAndSignUp*)arg;
	
    int command;
	char* username;
	char* password;
	char* ipAndPort;

    while(loginAndSign->status == true) {
        TCPSocket* sockfd_ready;

        MultipleTCPSocketsListener* listener = (MultipleTCPSocketsListener*)malloc(sizeof(MultipleTCPSocketsListener));
		pthread_mutex_lock(&loginAndSign->peers_mutex);
		memcpy(listener->sockets, loginAndSign->peers, loginAndSign->peers_count * sizeof(TCPSocket*));	 // copy pointer
		pthread_mutex_unlock(&loginAndSign->peers_mutex)

        sockfd_ready = listenToSocket(listener, loginAndSign->peers_count, 2);
        free(listener);

        if (sockfd_ready != NULL) {
			command = readCommand(sockfd_ready);
		}
		else
			continue;
        
        if (command == 0) {
			printf("error in ready peer");
			continue;
		}
		else if (command == LOGIN) {
			username = readMsg(sockfd_ready);
			password = readMsg(sockfd_ready);
			ipAndPort = destIpAndPort(sockfd_ready)

			if  (login(loginAndSign->dispatcher, username, password) == true) {
				writeCommand(sockfd_ready, CONFIRM_USER);
				writeMsg(sockfd_ready, username);
				writeMsg(sockfd_ready, ipAndPort);
				add_user(loginAndSign->dispatcher, sockfd_ready, username);
				remove_peer(loginAndSign, sockfd_ready);
				free(username);
				free(password);
				free(ipAndPort);
			}
			else {
				writeCommand(sockfd_ready,LOGIN_DENIED);
			}
		}
		else if (command == SIGNUP) {
			username = readMsg(sockfd_ready);
			password = readMsg(sockfd_ready);
			ipAndPort = destIpAndPort(sockfd_ready)

			if (signup(loginAndSign->dispatcher, username, password) == true) {
				writeCommand(sockfd_ready,CONFIRM_USER);
				writeMsg(sockfd_ready,username);
				writeMsg(sockfd_ready, ipAndPort);
				add_user(loginAndSign->dispatcher, sockfd_ready, username);
				add_all_users(loginAndSign->dispatcher, username);
				remove_peer(loginAndSign, sockfd_ready);
				free(username);
				free(password);
				free(ipAndPort);
			}
			else
			{
				writeCommand(sockfd_ready,USERNAME_TAKEN);
			}
		}
		else if (command == EXIT) {
			remove_peer(loginAndSign, sockfd_ready);
			cclose(sockfd_ready);
			free(sockfd_ready);
		}
		else {
			writeCommand(sockfd_ready,BAD_COMMAND);
		}
        
    }
}

void notifyPendingClients(LoginAndSignUp* loginAndSign) {
    if (loginAndSign == NULL) {
        return;
    }

    pthread_mutex_lock(&loginAndSign->peers_mutex);

    for (int i = 0; i < loginAndSign->peers_count; i++) {
        TCPSocket* peer = loginAndSign->peers[i];

        if (peer != NULL) {
            writeCommand(peer, CLOSE_CONNECTION);
        }
    }

    pthread_mutex_unlock(&loginAndSign->peers_mutex);
}

void closeLoginAndSignUp(LoginAndSignUp* loginAndSign) {
    if (loginAndSign == NULL) {
        return;
    }

	notifyPendingClients(loginAndSign);

    loginAndSign->status = false;

	pthread_mutex_lock(&loginAndSign->mutex);
    for (int i = 0; i < loginAndSign->peers_count; i++) {
        TCPSocket* peer = loginAndSign->peers[i];

        if (peer != NULL) {
            shutdown(peer->sockFd, SHUT_RDWR);
            close(peer->sockFd);
        }
    }

    if (loginAndSign->mthread != NULL && loginAndSign->status == true) {
        mt_wait(loginAndSign->mthread);
    }

    for (int i = 0; i < loginAndSign->peers_count; i++) {
        free(loginAndSign->peers[i]);
        loginAndSign->peers[i] = NULL;
    }
	pthread_mutex_unlock(&loginAndSign->mutex);

    loginAndSign->peers_count = 0;

    closeDispatcher(loginAndSign->dispatcher);
    free(loginAndSign->dispatcher);
    loginAndSign->dispatcher = NULL;

    free(loginAndSign->mthread);
    loginAndSign->mthread = NULL;
	pthread_mutex_destroy(&loginAndSign->peers_mutex);
}