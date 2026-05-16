#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdbool.h>

#include "MThread.h"
#include "TCPSocket.h"
#include "Peer2Peer.h"

typedef struct
{
	char* name; // userName của user muốn chat trong session
	char* ip;   // ip của user muốn chat trong session
	int port;   // port của user muốn chat trong session
} Partner;

typedef struct{
    TCPSocket* client_sock;
    char* client_name;
    bool connectionStatus;  // đã connect đến server thành công chưa 
    bool isLoggedIn;    // đã login chưa
    bool isInSession;   // đang trong session với user nào đó chưa
    MThread* mthread;
    Partner* partner;    // Thông tin đối tác trong session
    Peer2Peer* peer;    // socket dùng để chat trong session
} Client;

void initClient(Client* cli);
bool connectToServer(Client* cli, char* serverIp, int serverPort);
void run_client(void* arg);
void login(Client* cli, char* name,char* pass);
void signup(Client* cli, char* name, char* pass);
void openSession(Client* cli, char* username, char* peerUsr);
void closeSession(Client* cli);
void listAllUsers(Client* cli);
void getUsers(Client* cli);
void print_instructions();
void gotIpAndPort(Client* cli); // nhận tên, ip & port của user muốn chat từ server sau đó gán vào partner và set isInSession = true
void gotIncomingSession(Client* cli); // nhận tên, ip & port từ server khi có user khác mở session với mình
void clearPartner(Client* cli); // xóa thông tin partner 
void loggedIn(Client* cli);
void sendMsgToSession(Client* cli, char* msg);
void disconnectFromServer(Client* cli);
void closeApp(&cli);

#endif
