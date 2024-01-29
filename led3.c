#include <stdio.h>
#include <unistd.h>

#define LED_PATH "/sys/class/leds/gpio-led/brightness"
#define URANDOM_PATH "/dev/urandom"

int prenderLed(FILE* led_file, int value) {
    if (led_file == NULL) {
        perror("Error al abrir LED file");
        return 0;
    }

    if (value > 128) {
        fseek(led_file, 0, SEEK_SET);  
        fputc('1', led_file);
        printf("LED encendido.\n");
    } else {
        fseek(led_file, 0, SEEK_SET);  
        fputc('0', led_file);
        printf("LED apagado.\n");
    }

    fflush(led_file);

    return 1;
}

int main() {
    FILE *led_file = fopen(LED_PATH, "a");
    if (led_file == NULL) {
        perror("Error abriendo el archivo del LED");
        return 0;
    }

    FILE *urandom = fopen(URANDOM_PATH, "r");
    if (urandom == NULL) {
        perror("Error urandom");
        fclose(led_file);
        return 0;
    }

    while (1) {
        unsigned char random_value;
        if (fread(&random_value, sizeof(random_value), 1, urandom) != 1) {
            perror("Error urandom");
            fclose(urandom);
            fclose(led_file);
            return 0;
        }

        if (!prenderLed(led_file, random_value)) {
            fclose(urandom);
            fclose(led_file);
            return 0;
        }

        sleep(1);
    }

    fclose(urandom);
    fclose(led_file);

    return 0;
}
