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
	printf("%s%s\n","lu - print all users from serapp\n");
	printf("%s%s\n","lcu - print all connected users from serapp\n");
	printf("%s%s\n","o <username> - open a session with a uapp\n");
	printf("%s%s\n","cs - close the current sessapp\n");
	printf("%s%s\n","s <message> - send a message to session uapp\n");
	printf("%s%s\n","l - print connection staapp\n");
	printf("%s%s\n","d - disconnect from serapp\n");
	printf("%s%s\n","p - print instructiapp\n");
	printf("%s%s\n","x - close app\n");
}

void initClient(Client* cli) {
    cli->client_sock = NULL;
    cli->connectionStatus = false;
    cli->isLoggedIn = false;
    cli->isInSession = false;
    cli->mthread = (MThread*)malloc(sizeof(MThread));
	cli->partner = (Partner*)malloc(sizeof(Partner));
	cli->peer = (Peer2Peer*)malloc(sizeof(Peer2Peer));
}

void run_client(void* arg) {
    Client* cli = (Client*)arg;

    if (cli == NULL) {
        return;
    }

    printf("run client\n");
    int command;

    while (cli->connectionStatus)
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
				cli->isInSession = false;
				clearPartner(cli);
				break;
		}
	}
}

bool connectToServer(Client* cli, MThread* mthrd, char* serverIp, int serverPort) {
    if (cli == NULL || mthrd == NULL) {
        return false;
    }
    cli->client_sock = tcp_clent_create(serverIp, serverPort);

    if(cli->client_sock == NULL) {
        return false;
    }
    else {
        cli->connectionStatus = true;
        mthrd->run = run_client;
        mthrd->arg = (void*)cli;
        mt_start(mthrd);
    }
    return true;
}

void login(TCPSocket* socktowrite, char* name, char* pass) {
    writeCommand(socktowrite, LOGIN);
	writeMsg(socktowrite, name);
	writeMsg(socktowrite, pass);
}

void signup(TCPSocket* socktowrite, char* name, char* pass) {
    writeCommand(socktowrite, SIGNUP);
	writeMsg(socktowrite, name);
	writeMsg(socktowrite, pass);
}

void openSession(TCPSocket* socktowrite, char* username, char* peerUsr) {
	writeCommand(socktowrite, OPEN_SESSION_WITH_USER);
	writeMsg(socktowrite, username);
	writeMsg(socktowrite, peerUsr);
}

void listAllUsers(TCPSocket* socktowrite) {
	writeCommand(socktowrite, GET_ALL_CONNECTED_USERS);
}

void getUsers(TCPSocket* socktowrite) {
	writeCommand(socktowrite, GET_ALL_USERS);
}

clearPartner(Client* cli) {
	free(cli->partner->name);
	cli->partner->name = NULL;
	free(cli->partner->ip);
	cli->partner->ip = NULL;
	cli->partner->port = 0;
	cli->isInSession = false;
	printf("Clear Partner\n");
}

void gotIpAndPort(Client* cli) {
	char* msg;
	char* name;
	char *port;
	char *token;

	name = readMsg(cli->client_sock);	// userName của user muốn chat trong session
	msg = readMsg(cli->client_sock); 	// ip & port của user muốn chat trong session

	cli->partner->name = name;
	
	token = strtok(msg, ":");
	if(token != NULL) {
		cli->partner->ip = strdup(token);
	}

	token = strtok(NULL, ":");
	if(token != NULL) {
		cli->partner->port = atoi(token);
	}

	free(msg);

	printf("You are now in session with -> %s - %s:%d\n", cli->partner->name, cli->partner->ip, cli->partner->port);
	cli->isInSession = true;
}

void gotIncomingSession(Client* cli) {
	char *msg, *name;
	char *port;
	char *token;

	name = readMsg(cli->client_sock);	// userName của user muốn chat trong session
	msg = readMsg(cli->client_sock); 	// ip & port của user muốn chat trong session

	cli->partner->name = name;

	token = strtok(msg, ":");
	if(token != NULL) {
		cli->partner->ip = strdup(token);
	}

	token = strtok(NULL, ":");
	if(token != NULL) {
		cli->partner->port = atoi(token);
	}

	free(msg);

	printf("You are now in session with -> %s - %s:%d\n", cli->partner->name, cli->partner->ip, cli->partner->port);
	cli->isInSession = true;
}

void closeSession(Client* cli) {
	writeCommand(cli->client_sock, CLOSE_SESSION_WITH_USER);
	printf("session has ended\n");
}

void loggedIn(Client* cli) {	
	char *msg, *name;
	int int_port;
	char *token;

	cli->client_name = readMsg(cli->client_sock);	
	msg = readMsg(cli->client_sock); 	
	token = strtok(msg, ":");
	token = strtok(NULL, ":");

	int_port = atoi(token);
	cli->isLoggedIn = true;
	initPeer2Peer(cli->peer, int_port);
}

void sendMsgToSession(Client* cli, char* msg) {
	if(cli == NULL || msg == NULL || cli->peer == NULL || cli->partner->ip == NULL || cli->client_name == NULL) return;
	 
	if (cli->isInSession == true) {
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "[%s] %s", cli->client_name, msg);
		sendTo_udp(cli->peer, buffer, cli->partner->ip, cli->partner->port);
	}
	else {
		printf("You don't have an open session\n"); 
	}
}