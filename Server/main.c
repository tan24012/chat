#include "Server.h"

int main() {
    Server server ;

    initServer(&server);

    do {
        scanf("%s", command);

        if(strcmp(command, "x") == 0) {
            closeServer(&server);
        }
    } while(strcmp(command, "x") != 0);
}