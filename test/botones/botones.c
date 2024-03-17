#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#define NUM_BUTTONS 8 // Change this to the number of buttons connected

int main() {
    int fd;
    struct input_event ev;

    // Open the input device
    fd = open("/dev/input/event0", O_RDONLY);
    if (fd == -1) {
        perror("Error opening input device");
        return 1;
    }

    int button_pressed = 0;

    while (1) {
        // Read events
        if (read(fd, &ev, sizeof(struct input_event)) == -1) {
            perror("Error reading input event");
            close(fd);
            return 1;
        }
         

        // Check for button press event
        while(ev.type == EV_KEY && ev.value == 1){
            // Calculate the button number based on the event code
            int button_number = ev.code - KEY_RESERVED + 1;
            int temp_button = button_number;
            printf("Button %d has been pressed\n", button_number);
            printf("Value %d \n", ev.value);
            if(button_number != temp_button ){
                printf("Button %d has been released\n", button_number);
                break;
            }
            
        // }
        // if (ev.type == EV_KEY && ev.value == 1) {
        //     // Calculate the button number based on the event code
        //     int button_number = ev.code - KEY_RESERVED + 1;
        //     printf("Button %d has been pressed\n", button_number);
        // }
    }

    // Close the input device (This part of the code will never be reached since the while loop runs indefinitely)
    close(fd);

    return 0;
}
