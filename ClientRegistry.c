#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_MESSAGE 2048
#define MAX_LINE 1024

int main (int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int sockfd, n;
    struct sockaddr servaddr;
    socklen_t addrlen;
    fd_set rfds;
    char line[MAX_LINE], command[MAX_LINE], net[MAX_LINE], id[MAX_LINE], message[MAX_MESSAGE + 1];
    enum {notreg, regwait, reg, unregwait} state;

    // create the local socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // set hints to build socket information about server
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    // build socket information about server, including its IP address from its name
    getaddrinfo(argv[3], argv[4], &hints, &res);

    state = notreg;
    while(1){
        switch (state){
            case notreg:
            printf("Do you want to join or exit?\n");
            fgets(line, MAX_LINE, stdin);
            sscanf(line, "%s", command);
            if (strcmp(command, "join") == 0) {
                sscanf(line, "%*s%s%s", net, id); /*skip 1st and store 2nd and 3rd arguments*/

                // register node in the node server
                sprintf(message, "REG %s %s %s", net, argv[1], argv[2]);
                sendto(sockfd, message, strlen(message), 0, res->ai_addr, res->ai_addrlen);
                state = regwait;
            }
            else if (strcmp(command, "exit") == 0) {
                freeaddrinfo(res);
                close(sockfd);
                exit(0);
            }
            break; /* notreg */

            case regwait:
            printf("Waiting for confirmation from server. You can exit\n");
            FD_ZERO(&rfds); 
            FD_SET(0, &rfds); /* Waiting for keyboard input */
            FD_SET(sockfd, &rfds); /* Waiting for socket contact */
            n = select(sockfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *)NULL);
            if (FD_ISSET(sockfd, &rfds)) {

                // receive message from server
                addrlen = sizeof(servaddr);
                n = recvfrom(sockfd, message, MAX_MESSAGE, 0, &servaddr, &addrlen);
                message[n] = '\0';
                if (strcmp(message, "OKREG") == 0){
                    state = reg;
                    printf("Server Message: %s\n", message);
                }
            } else if (FD_ISSET(0, &rfds)) {
                fgets(line, MAX_LINE, stdin); /* Store the input from CL to line*/
                sscanf(line, "%s", command); /* Store the first value read in command*/
                if (strcmp(command, "exit") == 0) {
                    freeaddrinfo(res);
                    close(sockfd);
                    exit(0);
                }
            }
            break; /*regwait*/

            case reg:
            printf("Will you leave now?\n");
            fgets(line, MAX_LINE, stdin);
            sscanf(line, "%s", command);
            if (strcmp(command, "leave") == 0){

                // unregister the node
                sprintf(message, "%s %s %s %s", "UNREG", net, argv[1], argv[2]);
                n = sendto(sockfd, message, strlen(message), 0, res -> ai_addr, res ->ai_addrlen);
                state = unregwait;
            }
            break; /* reg */

            case unregwait:
            printf("Waiting for confirmation. You can exit\n");

            // wait either for confirmation of unreg or for exit
            FD_ZERO(&rfds);
            FD_SET(0, &rfds);
            FD_SET(sockfd, &rfds);
            n = select(sockfd + 1, &rfds, NULL, NULL, (struct timeval *) NULL);
            
            if (FD_ISSET(0, &rfds)){
                fgets(line, MAX_LINE, stdin);
                sscanf(line, "%s", command);
                if (strcmp(command, "exit") == 0){
                    freeaddrinfo(res);
                    close(sockfd);
                    exit(0);
                }

            } else if( FD_ISSET(sockfd, &rfds)){

                // receive confirmation of unreg
                addrlen = sizeof(servaddr);
                n = recvfrom(sockfd, message, MAX_MESSAGE, 0, &servaddr, &addrlen);
                message[n] = '\0';
                if (strcmp(message, "OKUNREG") == 0){
                    state = notreg;
                    printf("Ok, UNREG successful\n");
                }

            }
            break; /*unregwait*/

        }
    }
    exit(0);
}