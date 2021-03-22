#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define port "59000"

int main (void){
    int fd;
    ssize_t n;
    struct addrinfo hints, *res;

    char buffer[128];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM;

    n = getaddrinfo("tejo.tecnico.ulisboa.pt", port, &hints, &res);
    if (n != 0)
    {
        perror("Error: ");
        exit(1);
    }

    fd = socket(res-> ai_family, res->ai_socktype, 0);
    if (fd == -1)
    {
        perror("Error: ");
        exit(1);
    }

    n = connect(fd, res-> ai_addr, res-> ai_addrlen);
    if (n == -1)
    {
        perror("Error: ");
        exit(1);
    }

    n = write(fd, "Hello\n", 7);
    if (n == -1)
    {
        perror("Error: ");
        exit(1);
    }

    n = read(fd, buffer, 128);
    if (n == -1)
    {
        perror("Error: ");
        exit(1);
    }
    
    write(1, "echo: ", 6);
    write(1, buffer, n);
    freeaddrinfo(res);
    close(fd);
    
}