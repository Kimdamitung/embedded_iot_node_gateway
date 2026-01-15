#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>

#define SERIAL_PORT "/dev/ttyS3"
#define BAUDRATE    B9600
#define RX_BUF_SIZE 256

#define BUFFER  12
#define START   0x3C
#define NODE    0x4E
#define GATEWAY 0x47
#define END     0x24
#define NONE    0x00
#define C_ID    0x01
#define C_TIME  0x02

typedef struct {
    uint8_t start; // 0x3C (60)
    uint8_t header; // 0x4E (N), 0x47 (G)
    uint8_t id; // STT 0x01 to Node, 0x00 to Gateway 
    uint8_t commands;
    uint8_t data[6]; // temp 1 byte, humi 1 byte, hour 1 byte, minutes 1 byte, seconds 1 byte, reponse 1byte
    uint8_t length; //
    uint8_t end; // 0x24 (36)
} package_t;

static volatile sig_atomic_t running = 1;

void signal_handler(int sig){
    (void)sig;
    running = 0;
}

int uart_open(const char *dev){
    int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("open");
        return -1;
    }
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }
    cfsetispeed(&tty, BAUDRATE);
    cfsetospeed(&tty, BAUDRATE);
    /* 8N1 */
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_cflag &= ~CRTSCTS;
    /* RAW MODE */
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~OPOST;
    /* Non-blocking */
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }
    tcflush(fd, TCIOFLUSH);
    return fd;
}

void uart_tx_callback(int fd, package_t package){
    uint8_t tx_buffer[BUFFER];
    tx_buffer[0] = package.start;
    tx_buffer[1] = package.header;
    tx_buffer[2] = package.id;
    tx_buffer[3] = package.commands;
    tx_buffer[4] = package.data[0];
    tx_buffer[5] = package.data[1];
    tx_buffer[6] = package.data[2];
    tx_buffer[7] = package.data[3];
    tx_buffer[8] = package.data[4];
    tx_buffer[9] = package.data[5];
    tx_buffer[10] = package.length;
    tx_buffer[11] = package.end;
    size_t sent = 0;
    while (sent < BUFFER) {
        ssize_t n = write(fd, tx_buffer + sent, BUFFER - sent);
        if (n > 0) sent += n;
    }
}

int parse_package(const uint8_t *data){
    if(data[0] != START || data[1] != NODE || data[11] != END){
        return -1;
    }
    return data[3];
}

void uart_rx_callback(int fd){
    uint8_t buf[RX_BUF_SIZE];
    ssize_t n = read(fd, buf, sizeof(buf));
    if (n > 0) {
        printf("[RX] %zd bytes: ", n);
        for (ssize_t i = 0; i < n; i++) {
            printf("%02X ", buf[i]);
        }
        printf("\n");
    }
    if (n < BUFFER) return;
    package_t frame;
    time_t now;
    struct tm *t;
    switch (parse_package(buf)) {
        case NONE:
            printf("Data from frame %02X: ", buf[2]);
            printf("%02X - %02X - %02X - %02X - %02X - %02X\r\n", buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
            frame.start = START;
            frame.header = GATEWAY;
            frame.id = NONE;
            frame.commands = NONE;
            frame.data[0] = NONE;
            frame.data[1] = NONE;
            frame.data[2] = NONE;
            frame.data[3] = NONE;
            frame.data[4] = NONE;
            frame.data[5] = 1;
            frame.length = 6;
            frame.end = END;
            break;
        case C_ID:
            printf("C ID\r\n");
            frame.start = START;
            frame.header = GATEWAY;
            frame.id = 0x01;
            frame.commands = buf[3];
            frame.data[0] = NONE;
            frame.data[1] = NONE;
            frame.data[2] = NONE;
            frame.data[3] = NONE;
            frame.data[4] = NONE;
            frame.data[5] = 1;
            frame.length = 6;
            frame.end = END;
            break;
        case C_TIME:
            printf("C TIME\r\n");
            frame.start = START;
            frame.header = GATEWAY;
            frame.id = NONE;
            frame.commands = buf[3];
            time(&now);
            t = localtime(&now);
            frame.data[0] = NONE;
            frame.data[1] = NONE;
            frame.data[2] = t->tm_hour;
            frame.data[3] = t->tm_min;
            frame.data[4] = t->tm_sec;
            frame.data[5] = 1;
            frame.length = 6;
            frame.end = END;
            break;
        case (C_ID | C_TIME):
            printf("C ID | C TIME\r\n");
            frame.start = START;
            frame.header = GATEWAY;
            frame.id = 0x01;
            frame.commands = buf[3];
            time(&now);
            t = localtime(&now);
            frame.data[0] = NONE;
            frame.data[1] = NONE;
            frame.data[2] = t->tm_hour;
            frame.data[3] = t->tm_min;
            frame.data[4] = t->tm_sec;
            frame.data[5] = 1;
            frame.length = 6;
            frame.end = END;
            break;
        default:
            break;
    }
    printf("%02X - %02X - %02X - %02X - %02X - %02X - %02X - %02X - %02X - %02X - %02X - %02X\r\n", frame.start, frame.header, frame.id, frame.commands, frame.data[0], frame.data[1], frame.data[2], frame.data[3], frame.data[4], frame.data[5], frame.length, frame.end);
    uart_tx_callback(fd, frame);
}

void* uart_rx_thread(void *arg){
    int fd = *(int*)arg;
    fd_set rfds;
    struct timeval tv;

    printf("[UART] RX thread started\n");

    while (running) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(fd + 1, &rfds, NULL, NULL, &tv);

        if (ret < 0) {
            if (!running) break;
            perror("select");
            break;
        }

        if (ret == 0) {
            continue; // timeout
        }

        if (FD_ISSET(fd, &rfds)) {
            uart_rx_callback(fd);
        }
    }

    printf("[UART] RX thread exit\n");
    return NULL;
}

int main(void){
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    int fd = uart_open(SERIAL_PORT);
    if (fd < 0)
        return 1;

    pthread_t uart_tid;
    pthread_create(&uart_tid, NULL, uart_rx_thread, &fd);
    while (running) {
        sleep(1);
    }

    printf("[MAIN] stopping UART thread...\n");
    running = 0;

    close(fd);
    pthread_join(uart_tid, NULL);

    printf("[MAIN] exit\n");
    return 0;
}
