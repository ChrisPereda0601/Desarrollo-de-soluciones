#include <iostream>
#include <opencv2/opencv.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <cstdlib>
#include <sys/wait.h>
#include <alsa/asoundlib.h>
#include <fstream>
#include <string>
#include <sstream>

#define WIDTH   640
#define HEIGHT  480

using namespace cv;

constexpr int NUM_BUTTONS = 8;

int main() {
    const char* device = "/dev/input/event1"; // Cambia esto al dispositivo correcto
    int fd = open(device, O_RDONLY);
    if (fd == -1) {
        perror("Error opening device");
        exit(EXIT_FAILURE);
    }

    cv::Mat imagen;
    VideoCapture camara(0);

    camara.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
    camara.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);

    input_event ev;
    bool takePhoto = false;

    if (!camara.isOpened()) {
        std::cerr << "Error opening camera." << std::endl;
        exit(EXIT_FAILURE);
    }

    namedWindow("Imagen", WINDOW_AUTOSIZE);

    while (true) {
        // Check for button press
        

        if (read(fd, &ev, sizeof(input_event)) != -1) {
            if (ev.type == EV_KEY && ev.code == 412) {
                if (ev.value == 1) { // Botón presionado
                    std::cout << "Button 412 has been pressed, taking photo...\n";
                    takePhoto = true;
                }
            }
        }

        // Capture and display image
        camara >> imagen;
        if (imagen.empty()) {
            std::cerr << "No captured frame -- Break!" << std::endl;
            break;
        }
        imshow("Imagen", imagen);

        if (takePhoto) {
            std::cout << "Photo saved as 'captured_photo.jpg'." << std::endl;
            imwrite("captured_photo.jpg", imagen);
            takePhoto = false; // Reset flag

            usleep(5000000);

            system("./tflite_classification_example mobilenet_v1_1.0_224_quant.tflite labels_mobilenet_quant_v1_224.txt captured_photo.jpg");

            usleep(2000000);
            
            std::ifstream inFile("salida.txt");
            if (!inFile.is_open()) {
                std::cerr << "Error al abrir el archivo para lectura." << std::endl;
                return 1; // Retorna 1 para indicar error
            }

            std::string firstLine, firstWord;
            
            if (std::getline(inFile, firstLine)) { // Lee la primera línea del archivo
                std::istringstream iss(firstLine);
                iss >> firstWord;  // Extrae solo la primera palabra hasta el primer espacio
                
                // Construye el comando usando std::ostringstream
                std::ostringstream cmd;
                cmd << "aplay -Dplughw:1,0 -r 48000 -c 2 " << firstWord << ".wav -f S32_LE";
                
                std::cout << "Comando a ejecutar: " << cmd.str() << std::endl;

                system(cmd.str().c_str());
                //system("aplay -Dplughw:1,0 -r 48000 -c 2 client.wav -f S32_LE")
            } else {
                std::cout << "El archivo está vacío o no se pudo leer la primera línea." << std::endl;
            }



            inFile.close();
        }

        if ((char) waitKey(10) == 'c') {
            break; // Exit loop on 'c' key press
        }
    }



    camara.release();
    destroyAllWindows();
    close(fd);

    return 0;
}
