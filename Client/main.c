#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Client.h"
#include "Protocol.h"

int main(int argc, char* argv[]) {

    Client cli; // nhớ free mấy cái malloc khi đóng
    initClient(&cli);

    char command[100];
    char username[100];
    char password[100];
    char peerUsr[100];

    print_instructions();
    do {
        scanf("%s", command);

        if(strcmp(command, "c") == 0 && cli.shouldExit == false) {
            if(cli.connectionStatus == false) {
                if (!connectToServer(&cli, SERVER_IP, SERVER_PORT))
                    printf("connect failed\n");
                else 
                    printf("connected to %s:%d\n", SERVER_IP, SERVER_PORT);
            }
            else
                printf("Connected. Don't need to connect!\n");
        }
        else if(strcmp(command, "login") == 0 && cli.shouldExit == false) {
            printf("enter username: ");
            scanf("%s", username);
            printf("enter password: ");
            scanf("%s", password);

            if (cli.connectionStatus == false) {
				printf("ERROR - You are not connected to a server\n");
				continue;
			}
			else if(cli.isLoggedIn==false) {
				login(&cli, username, password);
				//sleep(5);
			}
			else
				printf("you are already logged in\n");
        }
        else if (strcmp(command, "register") == 0 && cli.shouldExit == false)  {
            printf("enter username: ");
            scanf("%s", username);
            printf("enter password: ");
            scanf("%s", password);

			if (cli.connectionStatus == false) {
				printf("ERROR - You are not connected to a server\n");
				continue;
			}
			
			signup(&cli, username, password);
		}
        else if (strcmp(command, "o") == 0 && cli.shouldExit == false) {
			printf("enter peer name: ");
            scanf("%s", peerUsr);

			if (cli.connectionStatus == false) {
				printf("ERROR - You are not connected to a server\n");
				continue;
			}
			else if(cli.isInSession==true ) {
				printf("ERROR - already in session with other user\n");
				continue;
			}
			else {
				openSession(&cli, username, peerUsr);
			}
		}
        else if (strcmp(command, "cs") == 0 && cli.shouldExit == false) 
		{
			if (cli.connectionStatus == false) {
				printf("ERROR - You are not connected to a server\n");
				continue;
			}
            else if(cli.isInSession == false) {
				printf("ERROR - You don't have an open session\n");
				continue;
			}
			closeSession(&cli);
		}
        else if (strcmp(command, "s") == 0 && cli.shouldExit == false) 
		{
			if (cli.connectionStatus == false) {
				printf("ERROR - You are not connected to a server\n");
				continue;
			}
			else if(cli.isInSession == true) {
				char name[100];
				// clear buffer
				int c;
				while ((c = getchar()) != '\n' && c != EOF);
				printf("enter: ");
				fgets(name, sizeof(name), stdin);
				name[strcspn(name, "\n")] = '\0';
				printf("%s\n", name);
                sendMsgToSession(&cli, name);
			}
			else
				printf("error\n");
		}
		else if (strcmp(command, "d") == 0 && cli.shouldExit == false) //disconnect from server
		{
			if (cli.connectionStatus == false)
			{
				printf("ERROR - You are not connected to a server\n");
				continue;
			}
			printf("Disconnecting Server...\n");
			disconnectFromServer(&cli, 1);
		}
		else if (strcmp(command, "x") == 0 || cli.shouldExit == true) //disconnect from server
		{
			if(cli.connectionStatus == true) {
				printf("Server closed\n");
				closeApp(&cli, 0);
			}
			else {
				printf("App closed\n");
				closeApp(&cli, 1);
				break;
			}
		}
    } while(strcmp(command, "x") != 0);

    return 0;
}
