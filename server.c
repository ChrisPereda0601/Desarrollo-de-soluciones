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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <sys/wait.h>
#include <alsa/asoundlib.h>

#define PORT 5000
#define BACKLOG 10
#define MAXSIZE 1024
#define NUM_BUTTONS 8 // Change this to the number of buttons connected
#define CHANNELS    2
#define FRAMES      768  

int recording = 0; // flag to indicate whether recording is in progress
snd_pcm_t *handle;

// Get the process ID of the current process

static void set_volume(long volume)
{
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "hw:1";
    const char *selem_name = "Softmaster";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);

    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
}


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

void listen_buttons(int fd, long volume) {
    struct input_event ev;
    pid_t pReproducir;
    long volume_actual = volume;
    set_volume(volume_actual);
    printf("Volumen actual: %ld\n",volume_actual);


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
                            record_audio("server.wav");
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
                    system("scp server.wav root@192.168.1.10:/home/root/chat");

                    // Encender LED en la otra tarjeta
                    system("ssh root@192.168.1.10 'echo 110 > /sys/class/leds/pca995x:\\green0/brightness'");

                    // Esperar 5 segundos
                    sleep(5);   

                    // Apagar LED en la otra tarjeta
                    system("ssh root@192.168.1.10 'echo 0 > /sys/class/leds/pca995x:\\green0/brightness'");
                }
            } else if (ev.code == 207 && ev.value == 1) { // Botón para reproducir
                printf("Button 207 has been pressed\n");
                printf("Reproducing audio\n");
                system("aplay -Dplughw:1,0 -r 48000 -c 2 client.wav -f S32_LE");  
            } else if(ev.code == 353 && ev.value == 1){
                //Subir volumen
                if(volume_actual >= 100){
                    printf("Solo puedes bajar volumen, volumen actual:%ld\n",volume_actual);
                }else{
                    volume_actual = volume_actual + 10;
                    printf("New Volume: %ld", volume_actual);
                    set_volume(volume_actual);
                }
            }else if(ev.code == 352 && ev.value == 1){
                //bajar volumen
                if(volume_actual <= 10){
                    printf("Solo puedes subir volumen, volumen actual:%ld\n",volume_actual);
                }else{
                    volume_actual = volume_actual - 10;
                    printf("New Volume: %ld", volume_actual);
                    set_volume(volume_actual);
                }
            }

        }
    }
}

int main() {
    struct sockaddr_in server;
    struct sockaddr_in dest;
    int status, socket_fd, client_fd, num;
    socklen_t size;

    pid_t p;

    char buffer[MAXSIZE];
    int yes = 1;

    pid_t parent_pid = getpid();
    pid_t pid1, pid2, pid3;

    long volume = 50;

    // Print the process ID
    printf("Process ID: %d\n", parent_pid);
    int fd = open("/dev/input/event0", O_RDONLY);
    if (fd == -1) {
        perror("Error opening input device");
        return 1;
    }

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



        pid1 = fork();
        // Verificamos si el fork fue exitoso
        if (pid1 < 0) {
            perror("Error forking process 1");
            exit(EXIT_FAILURE);
        } else if (pid1 == 0) {
            // Este es el proceso hijo 1
            printf("Child process 1 (PID: %d)\n", getpid());
            while(1){
                num = recv(client_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
                if (num > 0) {
                    buffer[num] = '\0';
                    printf("Server: Msg Received %s\n", buffer);
                }
                
            }
            exit(EXIT_SUCCESS); // Es importante terminar el proceso hijo
        }
        // Creamos el segundo proceso hijo
        pid2 = fork();

        // Verificamos si el fork fue exitoso
        if (pid2 < 0) {
            perror("Error forking process 2");
            exit(EXIT_FAILURE);
        } else if (pid2 == 0) {
            // Este es el proceso hijo 2
            printf("Child process 2 (PID: %d)\n", getpid());
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
            exit(EXIT_SUCCESS); // Es importante terminar el proceso hijo
        }
            // Creamos el tercer proceso hijo
        pid3 = fork();

        // Verificamos si el fork fue exitoso
        if (pid3 < 0) {
            perror("Error forking process 3");
            exit(EXIT_FAILURE);
        } else if (pid3 == 0) {
            // Este es el proceso hijo 3
            printf("Child process 3 (PID: %d)\n", getpid());
            listen_buttons(fd, volume);
            exit(EXIT_SUCCESS); // Es importante terminar el proceso hijo
        }
    


        // Close Connection Socket
        close(client_fd);
    }
    close(fd);
    close(socket_fd);
    return 0;
}
