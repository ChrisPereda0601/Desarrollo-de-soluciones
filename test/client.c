#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000
#define MAXSIZE 1024

int main(int argc, char *argv[]) {
    struct sockaddr_in server_info;
    struct hostent *he;
    int socket_fd, num;
    char buffer[MAXSIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: client hostname\n");
        exit(1);
    }

    if ((he = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "Cannot get host name\n");
        exit(1);
    }

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Socket Failure!!\n");
        exit(1);
    }

    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(PORT);
    memcpy(&server_info.sin_addr.s_addr, he->h_addr_list[0], he->h_length);

    if (connect(socket_fd, (struct sockaddr *)&server_info, sizeof(struct sockaddr)) < 0) {
        perror("connect");
        exit(1);
    }

    // Enter loop to continuously check for messages from server
    while (1) {
        // Check for incoming messages from server
        num = recv(socket_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
        if (num > 0) {
            buffer[num] = '\0';
            printf("Client: Message Received From Server - %s\n", buffer);
        }

        // Handle user input
        printf("Client: Enter Data for Server:\n");
        fgets(buffer, MAXSIZE - 1, stdin);
        if ((send(socket_fd, buffer, strlen(buffer), 0)) == -1) {
            fprintf(stderr, "Failure Sending Message\n");
            close(socket_fd);
            exit(1);
        }
    }

    close(socket_fd);
    return 0;
}
