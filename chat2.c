#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5566
#define IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[BUFFER_SIZE];

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        error("[-]Socket error");
    }
    printf("[+]TCP server socket created.\n");

    // Initialize server address
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    // Bind the socket to the address
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        error("[-]Bind error");
    }
    printf("[+]Bind to the port number: %d\n", PORT);

    // Listen for incoming connections
    listen(server_sock, 5);
    printf("Listening...\n");

    // Accept incoming connections
    addr_size = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(client_addr)); // Initialize client_addr
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
    if (client_sock < 0) {
        error("[-]Accept error");
    }
    printf("[+]Client connected.\n");

    // Communication loop
    while (1) {
        // Receive message from client
        bzero(buffer, BUFFER_SIZE);
        if (recv(client_sock, buffer, BUFFER_SIZE, 0) < 0) {
            error("[-]Receive error");
        }
        printf("Client: %s\n", buffer);

        // If client sends "quit", close the connection
        if (strcmp(buffer, "quit") == 0) {
            printf("Client has disconnected.\n");
            break;
        }

        // Send message to client
        printf("Server: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
        if (send(client_sock, buffer, strlen(buffer), 0) < 0) {
            error("[-]Send error");
        }

        // If server sends "quit", close the connection
        if (strcmp(buffer, "quit") == 0) {
            printf("Closing connection.\n");
            break;
        }
    }

    // Close sockets
    close(client_sock);
    close(server_sock);
    return 0;
}
