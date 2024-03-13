#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define BUFFER_SIZE 1024

// Estructura para pasar datos a los hilos
typedef struct {
    int socket;
} ThreadArgs;

// Función para recibir mensajes del otro usuario
void *receive_message(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int socket = args->socket;
    char buffer[BUFFER_SIZE];

    while (1) {
        int bytes_received = recv(socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Conexión terminada.\n");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("Mensaje recibido: %s\n", buffer);
    }

    close(socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <dirección IP>\n", argv[0]);
        return 1;
    }

    char *ip_address = argv[1];
    int server_socket, client_socket;
    struct sockaddr_in server_addr;
    pthread_t receive_thread;

    // Crear socket TCP
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    server_addr.sin_port = htons(PORT);

    // Conectar al servidor
    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error al conectar al servidor");
        exit(EXIT_FAILURE);
    }

    printf("Conexión establecida con el servidor.\n");

    // Iniciar el hilo para recibir mensajes
    ThreadArgs receive_args = {server_socket};
    if (pthread_create(&receive_thread, NULL, receive_message, (void *)&receive_args) != 0) {
        perror("Error al crear el hilo para recibir mensajes");
        exit(EXIT_FAILURE);
    }

    // Ciclo para enviar mensajes
    char message[BUFFER_SIZE];
    while (1) {
        printf("Ingrese un mensaje: ");
        fgets(message, BUFFER_SIZE, stdin);
        if (strcmp(message, "exit\n") == 0) {
            break;
        }
        send(server_socket, message, strlen(message), 0);
    }

    // Esperar a que el hilo de recepción termine
    pthread_join(receive_thread, NULL);

    // Cerrar el socket
    close(server_socket);

    return 0;
}
