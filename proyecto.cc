#include <iostream>
#include <opencv2/opencv.hpp>
#include <termios.h>
#include <unistd.h>

#define WIDTH   640
#define HEIGHT  480

using namespace cv;

// Función para configurar la terminal en modo no bloqueante
void setStdinEcho(bool enable = true) {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if (!enable) {
        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;
        tty.c_cc[VMIN] = 1;
        tty.c_cc[VTIME] = 0;
    } else {
        tty.c_lflag |= ICANON;
        tty.c_lflag |= ECHO;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

int main() {
    setStdinEcho(false);  // Desactivar eco y modo canónico

    cv::Mat image;
    VideoCapture camera(0);
    // if (!camera.isOpened()) {
    //     std::cerr << "Error opening camera." << std::endl;
    //     return -1;
    // }

    camera.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);

    namedWindow("en vivo", WINDOW_AUTOSIZE);

    while (true) {
        camera >> image;
        if (image.empty()) {
            std::cerr << "No frame captured from the camera." << std::endl;
            break;
        }
    
        cv::imshow("Live Feed", image);

        // if (waitKey(1) == 'c') {
        //     std::cout << "Photo saved as 'captured_photo.jpg'." << std::endl;
        //     imwrite("captured_photo.jpg", image);
        //     std::cout << "Photo saved as 'captured_photo.jpg'." << std::endl;
        // }

        // Leer de stdin en modo no bloqueante
        char ch;
        if (read(STDIN_FILENO, &ch, 1) > 0) {
            if (ch == 'q') {
                //break;
                std::cout << "Photo saved as 'captured_photo.jpg'." << std::endl;
                imwrite("captured_photo.jpg", image);
            }
        }
    }

    setStdinEcho(true);  // Restaurar configuración de la terminal
    camera.release();
    destroyAllWindows();
    return 0;
}
