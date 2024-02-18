#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#define PORT 3000
#define TIMES 10
#define SENSOR_ADDR 0x18

static int read_registers(int fd, uint8_t addr, uint8_t reg, uint8_t * buff, uint8_t size) {
    
    struct i2c_rdwr_ioctl_data msgs[1];
    struct i2c_msg i2cmsg[2];
    int ret;
    uint8_t register_address = reg | (1 << 7);

    /*Send WR Register Address*/
    i2cmsg[0].addr = addr;
    i2cmsg[0].flags = 0;
    i2cmsg[0].len = 1;
    i2cmsg[0].buf = &register_address;

    i2cmsg[1].addr = addr;
    i2cmsg[1].flags = I2C_M_RD; 
    i2cmsg[1].len = size;
    i2cmsg[1].buf = buff;

    msgs[0].msgs = i2cmsg;
    msgs[0].nmsgs = 2;

    if ( (ret = ioctl(fd, I2C_RDWR, msgs)) < 0) {
        perror("ERROR in I2C_RDWR");
        close(fd);
        return -1;
    }

    return 0;
}

static int read_accelerometer(int fd, float *x, float *y, float *z) {
    
    if (fd < 0) {
        perror("Path de I2C incorrecto");
        return -1;
    }

    
    if (ioctl(fd, I2C_SLAVE, SENSOR_ADDR) < 0) {
        perror("Error en I2C slave address");
        return -1;
    }

    // Read de X
    uint8_t x_data[2];
    if (read_registers(fd, SENSOR_ADDR, 0x28, x_data, sizeof(x_data)) != 0) {
        perror("Error al leer X");
        return -1;
    }
    //Casteo de 16 bits a float
    *x = (float)((int16_t)((x_data[1] << 8) | x_data[0])) / 1000.0;

    // Read de Y
    uint8_t y_data[2];
    if (read_registers(fd, SENSOR_ADDR, 0x2A, y_data, sizeof(y_data)) != 0) {
        perror("Error al leer Y");
        return -1;
    }
    //casteo de Y
    *y = (float)((int16_t)((y_data[1] << 8) | y_data[0])) / 1000.0;

    // Read de Z
    uint8_t z_data[2];
    if (read_registers(fd, SENSOR_ADDR, 0x2C, z_data, sizeof(z_data)) != 0) {
        perror("Error al leer Z");
        return -1;
    }
    //casteo de Z
    *z = (float)((int16_t)((z_data[1] << 8) | z_data[0])) / 1000.0;

    return 0;
}


static int write_registers(int fd, uint8_t addr, uint8_t reg, uint8_t * buff, uint8_t size) {

    char buffer[size + 1];

    if ( ioctl(fd, I2C_SLAVE, addr) < 0) {
        printf("ERROR in I2C_SLAVE\n");
        close(fd);
        return -1;
    }

    buffer[0] = reg;
    memcpy(&buffer[1], buff, size);

    /*Set Registers*/
    if (write(fd, &buffer, size + 1) < 0 ){
        perror("ERROR in WRITE\n");
        return -1;
    }
        
    return 0;
}

// activa xyz
static int activate_xyz_axis(int fd) {

    uint8_t value = 0x1F;
    uint8_t reg_address = 0x20;
    if (write_registers(fd, SENSOR_ADDR, reg_address, &value, 1) != 0) {
        return -1;
    }

    return 0;
}


int main(int argc, char * argv[]) {
    int socket_connection = socket(AF_INET, SOCK_STREAM, 0);

    //formato para el mensaje a enviar
    const char *msg = "x: %.2fg, y: %.2fg, z: %.2fg\n";
    char buffer[100];

    struct sockaddr_in server;
    memset(&server, 0 , sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(socket_connection, (struct sockaddr *)&server, sizeof(server));
    ret = listen(socket_connection, 10);

    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        perror("Error opening I2C device");
        return -1;
    }
    if (activate_xyz_axis(fd) != 0) {
        perror("Error en funcion activate XYZ axis");
        close(fd);
        return -1;
    }
    printf("XYZ acelerometro ACTIVADO\n");

    for(int i = 0; i < TIMES; i++) {
        int sock = accept(socket_connection, (struct sockaddr *)NULL, NULL);
        float x, y, z;
        ret = read_accelerometer(fd, &x, &y, &z);
        if (ret == 0) {
            printf("Leyendo XYZ data: x=%.2f, y=%.2f, z=%.2f\n", x, y, z);//imprime antes de enviar datos

            snprintf(buffer, sizeof(buffer), msg, x, y, z);
            ret = send(sock, buffer, strlen(buffer), 0);
            if (ret < 0) {
                perror("Error sending data over TCP");
            }
        } else {
            perror("Error reading accelerometer data");
        }
        ret = close(sock);
    }

    ret = close(socket_connection);
    close(fd);
    return 0;
}
