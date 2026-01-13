#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/select.h>

#define SERIAL_PORT "/dev/ttyS3"
#define BAUDRATE B9600
#define RX_BUF_SIZE 256

void uart_rx_callback(int fd){
    char buf[RX_BUF_SIZE];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("[CALLBACK] RX %zd bytes: %s\n", n, buf);
    }
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
    /* non-block read */
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

void uart_event_loop(int fd){
    fd_set rfds;
    printf("[INFO] UART event loop started\n");
    while (1) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        int ret = select(fd + 1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        }
        if (FD_ISSET(fd, &rfds)) {
            uart_rx_callback(fd);
        }
    }
}

int main(void)
{
    int fd = uart_open(SERIAL_PORT);
    if (fd < 0) return 1;
    /* SEND DATA  TX*/
    // const char *msg = "hello world\r\n";
    // write(fd, msg, strlen(msg));
    // tcdrain(fd);
    // printf("[TX] %s", msg);
    /* EVENT-DRIVEN RX */
    uart_event_loop(fd);
    close(fd);
    return 0;
}
