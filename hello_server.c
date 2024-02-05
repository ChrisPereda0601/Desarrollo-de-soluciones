#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 3000
#define TIMES 5

int main(int argc, char * argv[]) {

    const char * msg = "Hola desde Server\n\0";

    int socket_connection = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    memset(&server, 0 , sizeof(server));
    
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int ret = bind(socket_connection, (struct  sockaddr *)&server, sizeof(server) );
    ret = listen(socket_connection, 10);

    for(int i = 0; i < TIMES; i++) {
        int sock = accept(socket_connection, (struct sockaddr *)NULL, NULL);
        ret = send(sock, (void *)msg, strlen(msg), 0);
        ret = close(sock);
    }

    ret = close(socket_connection);
    return 0;
}