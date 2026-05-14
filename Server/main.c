#include "Server.h"

int main() {
    Server server ;

    initServer(&server);
    mt_wait(server.mthread);
}