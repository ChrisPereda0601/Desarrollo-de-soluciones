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
#include <alsa/asoundlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>


#define PORT 5000
#define MAXSIZE 1024


#define NUM_BUTTONS 8 // Change this to the number of buttons connected
#define CHANNELS    2
#define FRAMES      768  



int recording = 0; // flag to indicate whether recording is in progress
snd_pcm_t *handle;

void record_audio(char *filename) {
    FILE *rec_file = fopen(filename, "w");

    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);

    int ret;
    if ((ret = snd_pcm_open(&handle, "hw:0", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        printf("ERROR! Cannot open audio device\n");
        exit(EXIT_FAILURE);
    }

    if ((ret = snd_pcm_hw_params_any(handle, hw_params)) < 0) {
        printf("ERROR! Cannot configure audio parameters\n");
        exit(EXIT_FAILURE);
    }

    if ((ret = snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        printf("ERROR! Cannot set interleaved mode\n");
        exit(EXIT_FAILURE);
    }

    snd_pcm_format_t format = SND_PCM_FORMAT_S32_LE;
    if ((ret = snd_pcm_hw_params_set_format(handle, hw_params, format)) < 0) {
        printf("ERROR! Cannot set format\n");
        exit(EXIT_FAILURE);
    }

    int channels = CHANNELS;
    if ((ret = snd_pcm_hw_params_set_channels(handle, hw_params, channels)) < 0) {
        printf("ERROR! Cannot set channels\n");
        exit(EXIT_FAILURE);
    }

    int rate = 48000;
    if ((ret = snd_pcm_hw_params_set_rate_near(handle, hw_params, &rate, 0)) < 0) {
        printf("ERROR! Cannot set rate\n");
        exit(EXIT_FAILURE);
    }

    if ((ret = snd_pcm_hw_params(handle, hw_params)) < 0) {
        printf("ERROR! Cannot set hardware parameters\n");
        exit(EXIT_FAILURE);
    }

    uint32_t *buffer = (uint32_t *)malloc(CHANNELS * FRAMES * sizeof(uint32_t));

    while (recording) {
        snd_pcm_sframes_t frames = snd_pcm_readi(handle, buffer, FRAMES);
        if (frames < 0) {
            frames = snd_pcm_recover(handle, frames, 0);
            if (frames < 0) {
                printf("ERROR! Failed to recover from read error\n");
                exit(EXIT_FAILURE);
            }
        }
        int n_bytes = fwrite(buffer, 1, frames * CHANNELS * sizeof(uint32_t), rec_file);
        fflush(rec_file);
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    fclose(rec_file);
    exit(EXIT_SUCCESS);
}


void listen_buttons(int fd) {
    struct input_event ev;
    pid_t pReproducir;


    while (1) {
        if (read(fd, &ev, sizeof(struct input_event)) == -1) {
            perror("Error reading input event");
            exit(EXIT_FAILURE);
        }
        pid_t pGrabar;

        if (ev.type == EV_KEY) {
            if (ev.code == 412) { // Botón para grabar
                if (ev.value == 1) { // Botón presionado
                    printf("Button 412 has been pressed\n");
                    if (!recording) {
                        recording = 1;
                        pGrabar = fork();
                        if (pGrabar == 0) { // Proceso hijo
                            record_audio("client.wav");
                        } else if (pGrabar < 0) {
                            perror("Error forking process");
                            exit(EXIT_FAILURE);
                        }
                    }
                } else if (ev.value == 0) { // Botón liberado
                    printf("Button 412 has been released\n");
                    if (recording) {
                        printf("Recording stopped\n");
                        recording = 0;
                        kill(pGrabar, SIGINT);
                    }
                    //enviar audio al server
                    system("scp client.wav root@192.168.1.20:/home/root/chat"); 
                }
            } else if (ev.code == 207 && ev.value == 1) { // Botón para reproducir
                printf("Button 207 has been pressed\n");
                printf("Reproducing audio\n");
                system("aplay -Dplughw:1,0 -r 48000 -c 2 server.wav -f S32_LE");  
            }
        }
    }
}



int main(int argc, char *argv[]) {

    // inicio audio
        pid_t parent_pid = getpid();

    // Print the process ID
        printf("Process ID: %d\n", parent_pid);
        int fd = open("/dev/input/event0", O_RDONLY);
        if (fd == -1) {
            perror("Error opening input device");
            return 1;
        }

    //fin audio

 
    struct sockaddr_in server_info;
    struct hostent *he;
    int socket_fd, num;
    char buffer[MAXSIZE];

    pid_t p1, p2, p3;

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

        ///printf("Server got connection from client %s\n", inet_ntoa(dest.sin_addr));

        p1 = fork();

        if(p1<0){
             printf("Error en el proceso 1\n");
            return 1;
        }else if(p1==0){
            printf("Va a entrar al proceso 1\n");
            while(1){
                num = recv(socket_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                if (num > 0) {
                    buffer[num] = '\0';
                    printf("Client: Message Received From Server - %s\n", buffer);
                }
            }
        }

        p2 = fork();

        if(p2<0){
             printf("Error en el proceso 2\n");
            return 1;
        }else if(p2==0){
            printf("Va a entrar al proceso 2\n");
            while (1) {
                  // Handle user input
                printf("Client: Enter Data for Server:\n");
                fgets(buffer, MAXSIZE - 1, stdin);
                if ((send(socket_fd, buffer, strlen(buffer), 0)) == -1) {
                    fprintf(stderr, "Failure Sending Message\n");
                    close(socket_fd);
                    exit(1);
                }
            }
        } else {
            printf("Va a entrar al proceso padre\n");
            listen_buttons(fd);
        }
    }
    
    wait(NULL);
    wait(NULL);
    wait(NULL);

    close(fd);
    close(socket_fd);
    return 0;
}
