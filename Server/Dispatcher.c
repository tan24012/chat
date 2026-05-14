#include "Dispatcher.h"
#include "Protocol.h"

void initDispatcher(Dispatcher* dispatcher) {
	dispatcher->status = false;
    dispatcher->all_users_count = 0;
	dispatcher->login_users_count = 0;
	dispatcher->login_users_sockets_count = 0;
    dispatcher->mthread = (MThread*)malloc(sizeof(MThread));
    getAllUsers(dispatcher);
    pthread_mutex_init(&dispatcher->login_dispatcher_mutex, NULL);
}

void getAllUsers(Dispatcher* dispatcher) {
	FILE* file = fopen("/home/hoangtan/duytan/chat/Server/users.txt", "r");
	if(file == NULL) {
		printf("Error opening users.txt in get users\n");
		return;
	}

	char user[100];
    char pass[100];

	while(fscanf(file, "%s %s", user, pass) == 2) {
		dispatcher->all_users[dispatcher->all_users_count] = strdup(user);
		dispatcher->all_users_count++;
	}
	fclose(file);
}

bool checkLogined(Dispatcher* dispatcher, char* userName) {
    for(int i=0; i<dispatcher->login_users_count; i++) {
        if(strcmp(dispatcher->login_users[i]->name, userName) == 0) {
            return true;
        }
    }
    return false;
}

void add_user(Dispatcher* dispatcher, TCPSocket* sockfd, const char* username) {
	if (pthread_mutex_lock(&dispatcher->login_dispatcher_mutex) != 0)
	{
		printf("Error locking mutex");
		exit(1);
	}
	User* user = (User*)malloc(sizeof(User));

	user->socket = sockfd;
	user->name = username;
	user->connectionStatus = false;
	user->connectedToUser = NULL;

	dispatcher->login_users[dispatcher->login_users_count] = user;
	dispatcher->login_users_count++;
	dispatcher->login_users_sockets[dispatcher->login_users_sockets_count] = sockfd;
	dispatcher->login_users_sockets_count++;

	if (dispatcher->status == false)
	{
		dispatcher->status = true;
		dispatcher->mthread->run = runDispatcher;
		dispatcher->mthread->arg = (void*)dispatcher;
		mt_start(dispatcher->mthread);
	}
	if (pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex) != 0)
	{
		printf("Error unlocking mutex");
		exit(1);
	}
}

void add_all_users(Dispatcher* dispatcher, const char* username) {
	if (pthread_mutex_lock(&dispatcher->login_dispatcher_mutex) != 0)
	{
		printf("Error locking mutex");
		exit(1);
	}
	dispatcher->all_users[dispatcher->all_users_count] = username;
	dispatcher->all_users_count++;
	if (pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex) != 0)
	{
		printf("Error unlocking mutex");
		exit(1);
	}
}

void open_session(Dispatcher* dispatcher, User* user) {
	char *userName, *peerUsr, *user_ip_and_port, *target_ip_and_port;
	User* target = NULL;
	User* _user = NULL;

	userName = readMsg(user->socket); 	// userName của client muốn mở session
	peerUsr = readMsg(user->socket);	// userName của client đích

	for (int i=0; i<dispatcher->login_users_count; i++) {	// tìm User muốn mở sesion
		_user = dispatcher->login_users[i];
		if (user->socket == dispatcher->login_users[i]->socket) {
			break;
		}
	}
	
	for (int i=0; i<dispatcher->login_users_count; i++) {	// tìm User đích
		target = dispatcher->login_users[i];
		if (strcmp(dispatcher->login_users[i]->name, peerUsr) == 0) {
			break;
		}
	}

	if (target == NULL || _user == NULL || strcmp(target->name, _user->name) == 0) {	// 1 trong 2 user đang ko online, ko mở session với chính mình
		writeCommand(user->socket, OPEN_SESSION_ERROR);
		return ;
	}

	if ((_user->connectionStatus == false) && (target->connectionStatus==false)) {	// 2 user chưa mở session với ai mới có thể mở session với nhau
		target_ip_and_port = destIpAndPort(target->socket);	// gửi ip_and_port đích cho user muốn mở session
		writeCommand(user->socket, TARGET_IP_AND_PORT);
		writeMsg(user->socket, target->name);
		writeMsg(user->socket, target_ip_and_port);
		
		user_ip_and_port = destIpAndPort(_user->socket);	// gửi ip_and_port user muốn mở session cho đích
		writeCommand(target->socket, INCOMIMG_SESSION);
		writeMsg(target->socket, _user->name);
		writeMsg(target->socket, user_ip_and_port);

		target->connectionStatus = true;
		target->connectedToUser = _user;
		_user->connectionStatus = true;
		_user->connectedToUser = target;

					
		printf("Check session\n");
		printf("user name: %s\n", target->connectedToUser->name);
		printf("target name: %s\n", _user->connectedToUser->name);
		printf("target_ip_and_port: %s\n", target_ip_and_port);
		printf("user_ip_and_port: %s\n", user_ip_and_port);
	}
	else {
		writeCommand(user->socket,OPEN_CONNECTION_ERROR);
	}
}

void close_session(Dispatcher* dispatcher,  User* user)
{
	User* _user;
	bool targetExist = false;
	bool userExist = false;

	for (int i=0; i<dispatcher->login_users_count; i++) {	// tìm Client muốn đóng session
		_user = dispatcher->login_users[i];
		if (user->socket == dispatcher->login_users[i]->socket) {
			userExist = true;
			break;
		}
	}

	for (int i=0; i<dispatcher->login_users_count; i++) {	// tìm User đích còn hoạt động ko
		if (strcmp(dispatcher->login_users[i]->name, _user->connectedToUser->name) == 0) {
			targetExist = true;
			break;
		}
	}
	
	if (targetExist) {
		writeCommand(_user->connectedToUser->socket,SESSION_ENDED);
		_user->connectedToUser->connectedToUser = NULL;
		_user->connectedToUser->connectionStatus = false;
	}

	if (userExist) {
		writeCommand(_user->socket,SESSION_ENDED);
		_user->connectedToUser = NULL;
		_user->connectionStatus = false;
	}
	else {
		writeCommand(_user->socket,CLOSE_SESSION_ERROR);
	}
	printf("close session: %s\n", _user->name);
}

void runDispatcher(void* arg) {
	printf("Dispatcher is running...\n");

	Dispatcher* dispatcher = (Dispatcher*)arg;

	int command;

	while (dispatcher->status == true)
	{
		User* current_user = (User*)malloc(sizeof(User));
        MultipleTCPSocketsListener* listener = (MultipleTCPSocketsListener*)malloc(sizeof(MultipleTCPSocketsListener));
		memcpy(listener->sockets, dispatcher->login_users_sockets, dispatcher->login_users_sockets_count * sizeof(TCPSocket*));
		
		TCPSocket* temp_sock = listenToSocket(listener, dispatcher->login_users_count, 2);
		free(listener);

		if (temp_sock != NULL) {
			command = readCommand(temp_sock);
			current_user->socket = temp_sock; 
		}
		else
			continue;

		if (command == 0) {
			continue;
		}
		else if (command == OPEN_SESSION_WITH_USER) {
			open_session(dispatcher, current_user);
		}
		else if (command == CLOSE_SESSION_WITH_USER)
		{
			close_session(dispatcher, current_user);
		}
		// else if (command == EXIT) //user is exiting, update all chat rooms and users list
		// {
		// 	user_exit(current_user);
		// }
		// else if (command == GET_ALL_USERS) //all names from file
		// {
		// 	list_all_users(current_user);
		// }
		// else if (command == GET_ALL_CONNECTED_USERS) //send back to the user a list of all connected users
		// {
		// 	list_users(current_user);
		// }
	}
}