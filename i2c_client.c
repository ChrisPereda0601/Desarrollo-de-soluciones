#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h> // For exit()

#define PORT 3000
#define SIZE 100
#define TIMES 10

int main(int argc, char *argv[]) {
    char buff[SIZE];

    for (int i = 0; i < TIMES; i++) {
        int socket_connection = socket(AF_INET, SOCK_STREAM, 0);

        if (socket_connection == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in server;
        memset(&server, 0, sizeof(server));

        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        server.sin_addr.s_addr = htonl(INADDR_ANY); // Use INADDR_ANY for any available interface

        int ret = connect(socket_connection, (struct sockaddr *)&server, sizeof(server));

        if (ret == -1) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        ret = recv(socket_connection, (void *)buff, SIZE, 0);

        if (ret == -1) {
            perror("Receive failed");
            exit(EXIT_FAILURE);
        } else if (ret == 0) {
            printf("Server closed connection\n");
            exit(EXIT_SUCCESS);
        }

        buff[ret] = '\0'; // Null-terminate the received data

        printf("Datos del server: %s\n", buff);

        ret = close(socket_connection);

        if (ret == -1) {
            perror("Socket close failed");
            exit(EXIT_FAILURE);
        }

        sleep(1); // Add a delay between each iteration if needed
    }

    return 0;
}
