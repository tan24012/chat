#include "Client.h"
#include "Protocol.h"
#include "Common.h"

const char* MSG_FROM_SERVER =  "Got msg from server: ";
const char* EXIT_SESSION = "to exit current session or chat room enter cs\n";

void print_instructions()
{
	printf("%s%s\n","\nc <ip> - connect to serapp\n");
	printf("%s%s\n","login <username> <password> - Login to serapp\n");
	printf("%s%s\n","register <username> <password> - Signup to server\n");
	printf("%s%s\n","o <username> - open a session with a uapp\n");
	printf("%s%s\n","cs - close the current sessapp\n");
	printf("%s%s\n","d - disconnect from serapp\n");
	printf("%s%s\n","s <message> - send a message to session uapp\n");
	printf("%s%s\n","x - close app\n");
	printf("%s%s\n","l - print connection staapp\n");
	printf("%s%s\n","p - print instructiapp\n");
	printf("%s%s\n","lu - print all users from serapp\n");
	printf("%s%s\n","lcu - print all connected users from serapp\n");
	
}

void initClient(Client* cli) {
    cli->client_sock = NULL;	// lý do ko cấp phát
    cli->connectionStatus = false;
    atomic_store(&cli->isLoggedIn, false);
    atomic_store(&cli->isInSession, false);
	atomic_store(&cli->shouldExit, false);
    cli->mthread = (MThread*)malloc(sizeof(MThread));	// lý do cấp phát (làm cái gì và các hàm nào lưu thông tin vào cho struct đó, nếu ko cấp phát vùng nhớ thì truy cập vào các member sẽ ko hợp lệ --> segmentation default)
	mt_init(cli->mthread);
    cli->partner = (Partner*)malloc(sizeof(Partner));
	cli->peer = (Peer2Peer*)malloc(sizeof(Peer2Peer));
	pthread_mutex_init(&cli->partner_mutex, NULL);
}

void run_client(void* arg) {
    Client* cli = (Client*)arg;

    if (cli == NULL) {
        return;
    }

    printf("client running ...\n");
    int command;

    while(atomic_load(&cli->connectionStatus))
	{
		command = readCommand(cli->client_sock);

		if (!command)
			continue;

		switch(command)
		{
			case LOGIN_OR_SIGNUP:
				printf("%s%s\n","Connection established\nLogin or register to enter a chat room or establish a session");
				break;
			case LOGIN_DENIED:
				printf("%s%s\n",MSG_FROM_SERVER, "Bad username or password" );
				break;
			case USERNAME_TAKEN:
				printf("%s%s\n", MSG_FROM_SERVER, "The username you entered is already taken" );
				break;
			case CONFIRM_USER:
				printf("%s%s\n", MSG_FROM_SERVER, "You are now logged in" );
				loggedIn(cli);
				break;
			case TARGET_IP_AND_PORT:
				gotIpAndPort(cli);
				break;
			case INCOMIMG_SESSION:
				gotIncomingSession(cli);
				break;
			case SESSION_ENDED:
				atomic_store(&cli->isInSession, false);
				clearPartner(cli);
				break;
			case OPEN_SESSION_ERROR:
				atomic_store(&cli->isInSession, false);
				printf("%s%s\n", MSG_FROM_SERVER, "Error opening session ");
				break;
			case CLOSE_SESSION_ERROR:
				printf("%s%s\n", MSG_FROM_SERVER, "You are not in session");
				atomic_store(&cli->isInSession, false);
				break;
			case CLOSE_CONNECTION:
				printf("%s%s\n", MSG_FROM_SERVER, "Server was closed");
				atomic_store(&cli->shouldExit, true);
				break;
		}
	}
}

bool connectToServer(Client* cli, char* serverIp, int serverPort) {
    if (cli == NULL || cli->mthread == NULL || serverIp == NULL) {
        return false;
    }
    cli->client_sock = tcp_clent_create(serverIp, serverPort);

    if(cli->client_sock == NULL) {
        return false;
    }
    else {
        atomic_store(&cli->connectionStatus, true);
        cli->mthread->run = run_client;
        cli->mthread->arg = (void*)cli;
        mt_start(cli->mthread);
    }
    return true;
}

void login(Client* cli, char* name, char* pass) {
	if (cli == NULL || cli->client_sock == NULL || name == NULL || pass == NULL) {
        return;
    }
    writeCommand(cli->client_sock, LOGIN);
	writeMsg(cli->client_sock, name);
	writeMsg(cli->client_sock, pass);
}

void signup(Client* cli, char* name, char* pass) {
	if (cli == NULL || cli->client_sock == NULL || name == NULL || pass == NULL) {
        return;
    }
    writeCommand(cli->client_sock, SIGNUP);
	writeMsg(cli->client_sock, name);
	writeMsg(cli->client_sock, pass);
}

void openSession(Client* cli, char* username, char* peerUsr) {
	if (cli == NULL || cli->client_sock == NULL || username == NULL || peerUsr == NULL) {
        return;
    }
	writeCommand(cli->client_sock, OPEN_SESSION_WITH_USER);
	writeMsg(cli->client_sock, username);
	writeMsg(cli->client_sock, peerUsr);
}

void listAllUsers(Client* cli) {
	writeCommand(cli->client_sock, GET_ALL_CONNECTED_USERS);
}

void getUsers(Client* cli) {
	writeCommand(cli->client_sock, GET_ALL_USERS);
}

void clearPartner(Client* cli) {
    if (cli == NULL || cli->partner == NULL) {
        return;
    }

    pthread_mutex_lock(&cli->partner_mutex);

    free(cli->partner->name);
    cli->partner->name = NULL;

    free(cli->partner->ip);
    cli->partner->ip = NULL;

    cli->partner->port = 0;
    atomic_store(&cli->isInSession, false);

    pthread_mutex_unlock(&cli->partner_mutex);

    printf("Clear Partner\n");
}

void gotIpAndPort(Client* cli) {
	if (cli == NULL || cli->partner == NULL || cli->client_sock == NULL) {
		return;
	}
	char* msg;
    char* name;
    char* token;
    char* ip = NULL;
    int port = 0;

	name = readMsg(cli->client_sock);	// userName của user muốn chat trong session
	msg = readMsg(cli->client_sock); 	// ip & port của user muốn chat trong session

	if (name == NULL || msg == NULL) {
        return;
    }
	
	token = strtok(msg, ":");
	if(token != NULL) {
		ip = strdup(token);
	}

	token = strtok(NULL, ":");
	if(token != NULL) {
		port = atoi(token);
	}

	if (ip == NULL || port == 0) {
        free(name);
        free(ip);
        free(msg);
        return;
    }

	pthread_mutex_lock(&cli->partner_mutex);

    cli->partner->name = name;
    cli->partner->ip = ip;
    cli->partner->port = port;
    atomic_store(&cli->isInSession, true);

    pthread_mutex_unlock(&cli->partner_mutex);

    printf("You are now in session with -> %s - %s:%d\n", name, ip, port);

    free(msg);
}

void gotIncomingSession(Client* cli) {
    if (cli == NULL || cli->partner == NULL || cli->client_sock == NULL) {
        return;
    }

    char *msg = NULL;
    char *name = NULL;
    char *token = NULL;
    char *ip = NULL;
    int port = 0;

    name = readMsg(cli->client_sock);
    msg = readMsg(cli->client_sock);

    if (name == NULL || msg == NULL) {
        free(name);
        free(msg);
        return;
    }

    token = strtok(msg, ":");
    if (token != NULL) {
        ip = strdup(token);
    }

    token = strtok(NULL, ":");
    if (token != NULL) {
        port = atoi(token);
    }

    if (ip == NULL || port == 0) {
        free(name);
        free(ip);
        free(msg);
        return;
    }

    pthread_mutex_lock(&cli->partner_mutex);

    cli->partner->name = name;
    cli->partner->ip = ip;
    cli->partner->port = port;
    atomic_store(&cli->isInSession, true);

    printf("You are now in session with -> %s - %s:%d\n",
           cli->partner->name,
           cli->partner->ip,
           cli->partner->port);

    pthread_mutex_unlock(&cli->partner_mutex);

    free(msg);
}

void closeSession(Client* cli) {
	writeCommand(cli->client_sock, CLOSE_SESSION_WITH_USER);
	printf("session has ended\n");
}

void loggedIn(Client* cli) {
    if (cli == NULL || cli->client_sock == NULL || cli->peer == NULL) {
        return;
    }

    char *msg = NULL;
    char *name = NULL;
    char *token = NULL;
    int int_port = 0;

    name = readMsg(cli->client_sock);
    msg = readMsg(cli->client_sock);

    if (name == NULL || msg == NULL) {
        return;
    }

    token = strtok(msg, ":");
    token = strtok(NULL, ":");
    if (token == NULL) {
        free(name);
        free(msg);
        return;
    }

    int_port = atoi(token);
    if (int_port <= 0) {
        free(name);
        free(msg);
        return;
    }

    cli->client_name = name;
    atomic_store(&cli->isLoggedIn, true);

    free(msg);

    initPeer2Peer(cli->peer, int_port);
}

void sendMsgToSession(Client* cli, char* msg) {
	if(cli == NULL || msg == NULL || cli->partner->ip == NULL || cli->client_name == NULL) return;

	char ip[16];
    char name[100];
    int port = 0;
    Peer2Peer* peer = NULL;

	pthread_mutex_lock(&cli->partner_mutex);

	strncpy(ip, cli->partner->ip, sizeof(ip));
	strncpy(name, cli->client_name, sizeof(name));
	port = cli->partner->port;

	pthread_mutex_unlock(&cli->partner_mutex);
	 
	if (atomic_load(&cli->isInSession)) {
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "[%s] %s", cli->client_name, msg);
		sendTo_udp(cli->peer, buffer, ip, port);
	}
	else {
		printf("You don't have an open session\n"); 
	}
}

void disconnectFromServer(Client* cli, int exitCode) {
	if(cli == NULL || cli->client_sock == NULL) return;

	if(atomic_load(&cli->isLoggedIn)) {
		free(cli->client_name);
		cli->client_name = NULL;
		atomic_store(&cli->isLoggedIn, false);
	}

	if(atomic_load(&cli->isInSession)) {
		atomic_store(&cli->isInSession, false);
		clearPartner(cli);
		cclosePeer(cli->peer);	// đóng peer
	}

	atomic_store(&cli->connectionStatus, false);	// dừng thread
	if(exitCode == 1) {	// client chủ động
		writeCommand(cli->client_sock, EXIT);
	}
	cclose(cli->client_sock);
	mt_wait(cli->mthread);		
	free(cli->client_sock);
	cli->client_sock = NULL;
}

void closeApp(Client* cli, int exitCode) {
	if (cli == NULL) return;
	
	if(atomic_load(&cli->connectionStatus)) {
		disconnectFromServer(cli, exitCode);
	}

	free(cli->mthread);
	cli->mthread = NULL;

	pthread_mutex_lock(&cli->partner_mutex);

	free(cli->partner);
	cli->partner = NULL;

	pthread_mutex_unlock(&cli->partner_mutex);

	free(cli->peer);
	cli->peer = NULL;
}
