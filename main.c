#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int main (int argc, char *argv[]){
    struct addrinfo hints, *res;
    int sockfd, n;
    struct sockaddr servaddr;
    socklen_t addrlen;
    fd_set rfds;
    /*char line[MAX_LINE], command[MAX_LINE], net[MAX_LINE], id[MAX_LINE], message[MAX_MESSAGE + 1];*/
    
    // enumerate states
    enum{notregistered, registrating, registered, unregister, turnoff} state;

    // create the local socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // set hints to build socket information about server
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (argc < 5){ /*Inital 5 arguments not present*/
    printf("To launch the application there should be 5 arguments.\n");
                exit(0);
            }
    else if (argc == 5){
        /* build socket information about the server,
        including its IP address from its name*/
        getaddrinfo(argv[3], argv[4], &hints, &res);
        state = notregistered;
    }
}
while(1){
        switch (state){
            case notregistered:
            printf("Do you want to join or exit?\n");
            fgets(line, MAX_LINE, stdin);
            sscanf(line, "%s%s%s", command, net, id);
            if (strcmp(command, "exit") == 0){
                state = turnoff;
            }
            else if (strcmp(command, "join") == 0){
                // sscanf(line, "%*s%s%s", net, id);

                // register node in the server
                sprintf(message, "REG %s %s %s", net, argv[1], argv[2]);
                sendto(sockfd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen);
                state = registrating;
            }
            break; /*notregistered*/

            case registrating:
            printf("Waiting confirmation from server. You can exit\n");
            FD_ZERO(&rfds);
            FD_SET(0, &rfds); /*Waiting for keyboard input*/
            FD_SET(sockfd, &rfds); /*waiting for socket contact*/
            n = select(sockfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *)NULL);
            /*if n != 0 */

            if (n == -1){ /*connection error*/
            printf("Failed to register.\n");
            state = notregistered;
            }

            else if (FD_ISSET(0, &rfds)){
                fgets(line, MAX_LINE, stdin);
                sscanf(line, "%s", command);
                if (strcmp(command, "exit") == 0){
                    state = turnoff;
                }
            }
            
            else if (FD_ISSET(sockfd, &rfds)){

                /*receive message from server*/
                addrlen = sizeof(servaddr);
                n = recvfrom(sockfd, message, MAX_MESSAGE, 0, &servaddr, &addrlen); 
                /*error handling*/
                message[n]='\0';
                if (strcmp(message, "OKREG") == 0){
                    printf("Server Message: %s\n", message);
                    state=registered;
                }
            }
            break; /*Registrating*/
            
            case registered:
            printf("Waiting next command.\nYou can use leave to unregister the node,\nOr you can use exit.");
            /*You can use join to register another node,\n\*/
            fgets(line, MAX_LINE, stdin);
            sscanf(line, "%s", command);

            if (strcmp(command, "exit") == 0){
                state = turnoff;
            }
            else if (strcmp(command, "leave") == 0){
                sprintf(message, "%s %s %s %s", "UNREG", net, argv[1], argv[2]);
                n = sendto(sockfd, message, strlen(message), 0, res -> ai_addr, res ->ai_addrlen);
                if (n == -1){ /*connection error*/
                    printf("Failed to send message.\n");
                }
                state = unregister;
            }
            break; /*Registered*/

            case unregister:
            printf("Waiting for confirmation from server. You can exit\n");
            // wait either for confirmation of unreg or for exit
            FD_ZERO(&rfds);
            FD_SET(0, &rfds);
            FD_SET(sockfd, &rfds);
            n = select(sockfd + 1, &rfds, NULL, NULL, (struct timeval *)NULL);

            if (n == -1){ /*connection error*/
            printf("Failed to unregister.\n");
            }

            if (FD_ISSET(0, &rfds)){
                fgets(line, MAX_LINE, stdin);
                sscanf(line, "%s", command);
                if (strcmp(command, "exit") == 0){
                    state = turnoff;
                }
            }

            else if (FD_ISSET(sockfd, &rfds)){

                // receive confirmation of unreg
                addrlen = sizeof(servaddr);
                n = recvfrom(sockfd, message, MAX_MESSAGE, 0, &servaddr, &addrlen);
                message[n] = '\0';
                if (strcmp(message, "OKUNREG") == 0){
                    state = notregistered;
                    printf("Server Message: %s\n", message);
                }
            }
            break; /*Unregister*/

            case turnoff:
            printf("Exiting program.\n");
            freeaddrinfo(res);
            close(sockfd);
            exit(0);
            break; /*turnoff*/

        }
    }

}

