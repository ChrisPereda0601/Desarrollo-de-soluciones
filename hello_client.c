#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
 
#define PORT 3000
#define SIZE 100
 
int main(int argc, char * argv[]) {
 
    char buff[SIZE];
 
    int socket_connection = socket(AF_INET, SOCK_STREAM, 0);
 
    struct sockaddr_in server;
    memset(&server, 0 , sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = connect(socket_connection, (struct sockaddr *)&server, sizeof(server));
    ret = recv(socket_connection, (void *)buff, SIZE, 0);

    printf("Rx: %d\n", ret);
    printf("%s", buff);
 
    ret = close(socket_connection);

    return 0;

}
