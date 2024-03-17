#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#define NUM_BUTTONS 8 // Change this to the number of buttons connected

int main() {
    int fd;
    struct input_event ev;

    // Abre el dispositivo de entrada
    fd = open("/dev/input/event0", O_RDONLY);
    if (fd == -1) {
        perror("Error opening input device");
        return 1;
    }

    int is_recording = 0; // Flag para indicar si se está grabando

    while (1) {
        // Lee eventos
        if (read(fd, &ev, sizeof(struct input_event)) == -1) {
            perror("Error reading input event");
            close(fd);
            return 1;
        }

        // Verifica el evento de presionar botón
        if (ev.type == EV_KEY) {
            int button_number = ev.code - KEY_RESERVED + 1;
            if (ev.value == 1) { // Botón presionado
                printf("Button %d has been pressed\n", button_number);
                if (!is_recording) {
                    // Iniciar grabación de audio aquí
                    printf("Recording started\n");
                    is_recording = 1;
                }
            } else if (ev.value == 0) { // Botón liberado
                printf("Button %d has been released\n", button_number);
                if (is_recording) {
                    // Finalizar grabación de audio y enviar al destino aquí
                    printf("Recording stopped and sent to destination\n");
                    is_recording = 0;
                }
            }
        }
    }

    // Close the input device
    close(fd);

    return 0;
}
