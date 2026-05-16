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
    cli->isLoggedIn = false;
    cli->isInSession = false;
    cli->mthread = (MThread*)malloc(sizeof(MThread));	// lý do cấp phát (làm cái gì và các hàm nào lưu thông tin vào cho struct đó, nếu ko cấp phát vùng nhớ thì truy cập vào các member sẽ ko hợp lệ --> segmentation default)
	cli->partner = (Partner*)malloc(sizeof(Partner));
	cli->peer = (Peer2Peer*)malloc(sizeof(Peer2Peer));
}

void run_client(void* arg) {
    Client* cli = (Client*)arg;

    if (cli == NULL) {
        return;
    }

    printf("client running ...\n");
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

bool connectToServer(Client* cli, char* serverIp, int serverPort) {
    if (cli == NULL || cli->mthread == NULL) {
        return false;
    }
    cli->client_sock = tcp_clent_create(serverIp, serverPort);

    if(cli->client_sock == NULL) {
        return false;
    }
    else {
        cli->connectionStatus = true;
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
	if (cli == NULL || cli->partner == NULL || cli->client_sock == NULL) {
		return;
	}
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
	if (cli == NULL || cli->partner == NULL || cli->client_sock == NULL) {
		return;
	}
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
	if (cli == NULL || cli->client_sock == NULL || cli->peer == NULL) {
        return;
    }
	char *msg, *name;
	int int_port;
	char *token;

	cli->client_name = readMsg(cli->client_sock);	
	msg = readMsg(cli->client_sock); 	
	token = strtok(msg, ":");
	token = strtok(NULL, ":");
	int_port = atoi(token);
	free(msg);

	cli->isLoggedIn = true;
	initPeer2Peer(cli->peer, int_port);
}

void sendMsgToSession(Client* cli, char* msg) {
	if(cli == NULL || msg == NULL || cli->partner->ip == NULL || cli->client_name == NULL) return;
	 
	if (cli->isInSession == true) {
		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "[%s] %s", cli->client_name, msg);
		sendTo_udp(cli->peer, buffer, cli->partner->ip, cli->partner->port);
	}
	else {
		printf("You don't have an open session\n"); 
	}
}

void disconnectFromServer(Client* cli) {
	if(cli == NULL || cli->client_sock == NULL) return;

	if(cli->isLoggedIn == true) {
		free(cli->client_name);
		cli->client_name = NULL;

		cclosePeer(cli->peer);	// đóng peer

		cli->isLoggedIn == false;
	}

	if(cli->isInSession == true) {
		clearPartner(cli);
		cli->isInSession = false;
	}

	cli->connectionStatus = false;	// dừng thread
	writeCommand(cli->client_sock, EXIT);
	cclose(cli->client_sock);
	mt_wait(cli->mthread);		
	free(cli->client_sock);
	cli->client_sock = NULL;
}

void closeApp(Client* cli) {
	if (cli == NULL) return;
	
	if(cli->connectionStatus == true) {
		disconnectFromServer(cli);
	}
	else {
        cli->connectionStatus = false;
        if (cli->mthread != NULL) {
            mt_wait(cli->mthread);
        }
    }

	free(cli->mthread);
	cli->mthread = NULL;
	free(cli->partner);
	cli->partner = NULL;
	free(cli->peer);
	cli->peer = NULL;
}