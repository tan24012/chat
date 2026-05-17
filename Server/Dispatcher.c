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
	pthread_mutex_lock(&dispatcher->login_dispatcher_mutex);
    for(int i=0; i<dispatcher->login_users_count; i++) {
        if(strcmp(dispatcher->login_users[i]->name, userName) == 0) {
            return true;
        }
    }
	pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex);
    return false;
}

void add_user(Dispatcher* dispatcher, TCPSocket* sockfd, const char* username) {
	pthread_mutex_lock(&dispatcher->login_dispatcher_mutex);

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
	pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex);
}

void add_all_users(Dispatcher* dispatcher, const char* username) {
	pthread_mutex_lock(&dispatcher->login_dispatcher_mutex);
	dispatcher->all_users[dispatcher->all_users_count] = username;
	dispatcher->all_users_count++;
	pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex);
}

void open_session(Dispatcher* dispatcher, User* user) {
	char *userName, *peerUsr, *user_ip_and_port, *target_ip_and_port;
	User* target = NULL;
	User* _user = NULL;

	userName = readMsg(user->socket); 	// userName của client muốn mở session
	peerUsr = readMsg(user->socket);	// userName của client đích

	pthread_mutex_lock(&dispatcher->login_dispatcher_mutex);
	for (int i=0; i<dispatcher->login_users_count; i++) {	// tìm User muốn mở sesion coi đã login chưa
		if (user->socket == dispatcher->login_users[i]->socket) {
			_user = dispatcher->login_users[i];
			break;
		}
	}
	
	for (int i=0; i<dispatcher->login_users_count; i++) {	// tìm User đích
		if (strcmp(dispatcher->login_users[i]->name, peerUsr) == 0) {
			target = dispatcher->login_users[i];
			break;
		}
	}

	if (target == NULL || _user == NULL || strcmp(target->name, _user->name) == 0) {	// ko mở session với chính mình
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

		pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex);

		printf("Check session\n");
		printf("user name: %s\n", target->connectedToUser->name);
		printf("target name: %s\n", _user->connectedToUser->name);
		printf("target_ip_and_port: %s\n", target_ip_and_port);
		printf("user_ip_and_port: %s\n", user_ip_and_port);
	}
	else {
		writeCommand(user->socket,OPEN_SESSION_ERROR);
	}

	free(userName);
	free(peerUsr);
}

void close_session(Dispatcher* dispatcher,  User* user)
{
	User* _user;
	bool targetExist = false;
	bool userExist = false;

	pthread_mutex_lock(&dispatcher->login_dispatcher_mutex);
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
	pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex);
	printf("close session: %s\n", _user->name);
}

void user_exit(User* current_user) {
	User* user;
	TCPSocket* login_sock;
	int index = -1;

	pthread_mutex_lock(&dispatcher->login_dispatcher_mutex);

	for (int i = 0; i < dispatcher->login_users_count; i++) {
        if (dispatcher->login_users[i]->socket == current_user->socket) {
            user = dispatcher->login_users[i];
            index = i;
            break;
        }
    }
	for (int i = index; i < dispatcher->login_users_count - 1; i++) {
        dispatcher->login_users[i] = dispatcher->login_users[i + 1];
    }
	dispatcher->login_users_count--;

	for (int i = 0; i < dispatcher->login_users_sockets_count; i++) {
        if (dispatcher->login_users_sockets[i] == current_user->socket) {
            login_sock = dispatcher->login_users_sockets[i];
            index = i;
            break;
        }
    }
	for (int i = index; i < dispatcher->login_users_sockets_count - 1; i++) {
        dispatcher->login_users_sockets[i] = dispatcher->login_users[i + 1];
    }
	dispatcher->login_users_sockets_count--;

	pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex);
	
	if(user->connectedToUser != NULL) {
		close_session(dispatcher, current_user);
		writeCommand(user->connectedToUser->socket,SESSION_ENDED);
	}

	cclose(user->socket);
	free(user->socket);
	free(user);
}

void runDispatcher(void* arg) {
	printf("Dispatcher is running...\n");

	Dispatcher* dispatcher = (Dispatcher*)arg;

	int command;

	while (dispatcher->status == true)
	{
        MultipleTCPSocketsListener* listener = (MultipleTCPSocketsListener*)malloc(sizeof(MultipleTCPSocketsListener));
		pthread_mutex_lock(&dispatcher->login_dispatcher_mutex);
		memcpy(listener->sockets, dispatcher->login_users_sockets, dispatcher->login_users_sockets_count * sizeof(TCPSocket*));
		pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex);

		TCPSocket* temp_sock = listenToSocket(listener, dispatcher->login_users_count, 2);
		free(listener);

		if (temp_sock != NULL) {
			User* current_user = (User*)malloc(sizeof(User));
			command = readCommand(temp_sock);
			current_user->socket = temp_sock; 
		}
		else 
			continue;

		if (command == 0) {
			free(current_user);
			continue;
		}
		else if (command == OPEN_SESSION_WITH_USER) {
			open_session(dispatcher, current_user);
		}
		else if (command == CLOSE_SESSION_WITH_USER) {
			close_session(dispatcher, current_user);
		}
		else if (command == EXIT)  {
			user_exit(current_user);
		}
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

void notifyDispatcherClients(Dispatcher* dispatcher) {
    if (dispatcher == NULL) {
        return;
    }

    pthread_mutex_lock(&dispatcher->login_dispatcher_mutex);

    for (int i = 0; i < dispatcher->login_users_count; i++) {
        User* user = dispatcher->login_users[i];

        if (user != NULL && user->socket != NULL) {
            writeCommand(user->socket, CLOSE_CONNECTION);
        }
    }

    pthread_mutex_unlock(&dispatcher->login_dispatcher_mutex);
}

void closeDispatcher(Dispatcher* dispatcher) {
    if (dispatcher == NULL) {
        return;
    }

	notifyDispatcherClients(dispatcher);

    dispatcher->status = false;

    for (int i = 0; i < dispatcher->login_users_count; i++) {
        User* user = dispatcher->login_users[i];

        if (user != NULL && user->socket != NULL) {
            writeCommand(user->socket, CLOSE_CONNECTION);
            shutdown(user->socket->sockFd, SHUT_RDWR);
            close(user->socket->sockFd);
        }
    }

    if (dispatcher->mthread != NULL) {
        mt_wait(dispatcher->mthread);
    }

    for (int i = 0; i < dispatcher->login_users_count; i++) {
        User* user = dispatcher->login_users[i];

        if (user != NULL) {
            free(user->socket);
            user->socket = NULL;

            free(user->name);
            user->name = NULL;

            free(user);
            dispatcher->login_users[i] = NULL;
        }
    }

    for (int i = 0; i < dispatcher->login_users_sockets_count; i++) {
        dispatcher->login_users_sockets[i] = NULL;
    }

    dispatcher->login_users_count = 0;
    dispatcher->login_users_sockets_count = 0;

    pthread_mutex_destroy(&dispatcher->login_dispatcher_mutex);

    free(dispatcher->mthread);
    dispatcher->mthread = NULL;
}