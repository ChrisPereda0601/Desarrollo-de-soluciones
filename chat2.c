#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 3000
#define SIZE 1024

int main(int argc, char *argv[]) {
    int socket_connection;
    struct sockaddr_in server;

    // Check if IP address is provided as a command-line argument
    if (argc < 2) {
        printf("Usage: %s <IP_ADDRESS>\n", argv[0]);
        return 1;
    }

    // Create socket
    socket_connection = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_connection < 0) {
        perror("socket");
        return 1;
    }

    // Set up server address structure
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(argv[1]);

    // Connect to the server
    if (connect(socket_connection, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        return 1;
    }

    // If the IP address is the local one, act as server
    if (strcmp(argv[1], "127.0.0.1") == 0 || strcmp(argv[1], "localhost") == 0) {
        printf("Running as server...\n");
        // Listen for incoming connections
        int client_socket, client_length;
        struct sockaddr_in client;
        char buffer[SIZE];

        if (listen(socket_connection, 5) < 0) {
            perror("listen");
            return 1;
        }

        // Accept connection from client
        client_length = sizeof(client);
        client_socket = accept(socket_connection, (struct sockaddr *)&client, (socklen_t *)&client_length);
        if (client_socket < 0) {
            perror("accept");
            return 1;
        }

        // Receive data from client
        ssize_t bytes_received = recv(client_socket, buffer, SIZE, 0);
        if (bytes_received < 0) {
            perror("recv");
            return 1;
        }

        buffer[bytes_received] = '\0';
        printf("Received message: %s", buffer);

        // Close client socket
        close(client_socket);
    } else {  // Act as client
        printf("Running as client...\n");
        char message[SIZE];

        printf("Enter message to send: ");
        fgets(message, SIZE, stdin);

        // Send message to server
        if (send(socket_connection, message, strlen(message), 0) < 0) {
            perror("send");
            return 1;
        }
    }

    // Close socket
    close(socket_connection);

    return 0;
}
