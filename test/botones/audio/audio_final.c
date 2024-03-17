#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <alsa/asoundlib.h>
#include <signal.h>

#define NUM_BUTTONS 8 // Change this to the number of buttons connected
#define CHANNELS    2
#define FRAMES      768  

int recording_pid = -1; // PID of the recording process
int is_recording = 0;   // flag to indicate whether recording is in progress
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

    while (is_recording) {
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

void stop_recording() {
    if (recording_pid != -1) {
        kill(recording_pid, SIGINT);
        recording_pid = -1;
    }
}

void listen_buttons(int fd) {
    struct input_event ev;

    while (1) {
        if (read(fd, &ev, sizeof(struct input_event)) == -1) {
            perror("Error reading input event");
            exit(EXIT_FAILURE);
        }

        if (ev.type == EV_KEY) {
            int button_number = ev.code - KEY_RESERVED + 1;
            if (ev.value == 1) { // Button pressed
                printf("Button %d has been pressed\n", button_number);
                if (ev.code == 412 && !is_recording) { // Start recording when button 412 is pressed
                    is_recording = 1;
                    pid_t pid = fork();
                    if (pid == 0) { // Child process
                        record_audio("test.wav");
                    } else if (pid < 0) {
                        perror("Error forking process");
                        exit(EXIT_FAILURE);
                    } else {
                        recording_pid = pid;
                    }
                }
            } else if (ev.value == 0) { // Button released
                printf("Button %d has been released\n", button_number);
                if (ev.code == 412 && is_recording) { // Stop recording when button 412 is unpressed
                    is_recording = 0;
                    stop_recording();
                }
            }
        }
    }
}

int main() {
    int fd = open("/dev/input/event0", O_RDONLY);
    if (fd == -1) {
        perror("Error opening input device");
        return 1;
    }

    listen_buttons(fd);

    close(fd);

    return 0;
}
