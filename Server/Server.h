
#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "TCPSocket.h"
#include "Protocol.h"
#include "LoginAndSignUp.h"

typedef struct {
    TCPSocket* listen_sock;
    LoginAndSignUp* loginAndSign;
    MThread* mthread;
    atomic_bool status;
} Server;

void initServer(Server* serv);
void runServer(void* arg);
void closeServer(Server* serv);

#endif