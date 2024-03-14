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
#define BACKLOG 10
#define MAXSIZE 1024

int main() {
    struct sockaddr_in server;
    struct sockaddr_in dest;
    int status, socket_fd, client_fd, num;
    socklen_t size;

    pid_t p;

    char buffer[MAXSIZE];
    int yes = 1;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    memset(&server, 0, sizeof(server));
    memset(&dest, 0, sizeof(dest));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if ((bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr))) == -1) {
        fprintf(stderr, "Binding Failure\n");
        exit(1);
    }

    if ((listen(socket_fd, BACKLOG)) == -1) {
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }



    while (1) {
        size = sizeof(struct sockaddr_in);
        if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size)) == -1) {
            perror("accept");
            exit(1);
        }

        printf("Server got connection from client %s\n", inet_ntoa(dest.sin_addr));

        p = fork();

        if(p<0){
            printf("Ana maria viene de malas\n");
            return 1;
        }else if(p==0){
            //proceso hijo
            while(1){
                num = recv(client_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                if (num > 0) {
                    buffer[num] = '\0';
                    printf("Server: Msg Received %s\n", buffer);
                }
            }
        }
        else{
            //proceso PADRE
            printf("Va a entrar al padre\n");
            while (1) {
                // Check for incoming messages from client


                // Handle server-side input
                // For demonstration purposes, let's read from stdin and send it to the client
                printf("Server: Enter Data for Client:\n");
                fgets(buffer, MAXSIZE - 1, stdin);
                if ((send(client_fd, buffer, strlen(buffer), 0)) == -1) {
                    fprintf(stderr, "Failure Sending Message\n");
                    close(client_fd);
                    exit(1);
                }
            }
        }
        // Enter loop to continuously check for messages from client


        // Close Connection Socket
        close(client_fd);
    }

    close(socket_fd);
    return 0;
}
