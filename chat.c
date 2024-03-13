#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 3000
#define MAX_PENDING 5
#define MAX_MSG_SIZE 1024

void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char message[MAX_MSG_SIZE];

    printf("Cliente conectado desde %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Leer y enviar mensajes
    while (1) {
        ssize_t bytes_received = recv(client_socket, message, MAX_MSG_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }
        message[bytes_received] = '\0';
        printf("Cliente: %s", message);

        printf("Servidor: ");
        fgets(message, MAX_MSG_SIZE, stdin);
        send(client_socket, message, strlen(message), 0);
    }

    // Cerrar el socket del cliente
    close(client_socket);
    printf("Cliente desconectado\n");
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Crear el socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Vincular el socket a la dirección y puerto
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error en la vinculación");
        exit(EXIT_FAILURE);
    }

    // Escuchar por conexiones entrantes
    if (listen(server_socket, MAX_PENDING) == -1) {
        perror("Error al intentar escuchar");
        exit(EXIT_FAILURE);
    }

    printf("Servidor esperando conexiones en el puerto %d...\n", PORT);

    // Aceptar conexiones entrantes y manejarlas
    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
            perror("Error al aceptar la conexión entrante");
            exit(EXIT_FAILURE);
        }

        // Crear un nuevo proceso para manejar la conexión
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error al crear un nuevo proceso");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Proceso hijo
            close(server_socket); // El proceso hijo no necesita el socket del servidor
            handle_client(client_socket, client_addr);
            exit(EXIT_SUCCESS);
        } else { // Proceso padre
            close(client_socket); // El proceso padre no necesita el socket del cliente
            wait(NULL); // Esperar al proceso hijo para evitar zombies
        }
    }

    // Cerrar el socket del servidor (nunca se llega aquí)
    close(server_socket);

    return 0;
}
