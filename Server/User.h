

#ifndef USER_H_
#define USER_H_

#include "Common.h"
#include <stdbool.h>

typedef struct User User;

struct User {
    TCPSocket* socket;
    char* name;
    bool connectionStatus;  // false: chưa mở session với ai, true: đã mở session với 1 user nào đó
    User* connectedToUser;
};


#endif /* USER_H_ */
