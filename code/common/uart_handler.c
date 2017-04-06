#include "uart_handler.h"

static int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

static void set_mincount(int fd, int mcount, int timeout)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    /* if mcount == 0, timed read with 500ms */
    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = timeout;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}


/* 
 * Methods from Arduino compatible:
 *  - begin(port)
 *  - available(port)
 *  - readBytesUntil(port)  
 */

/* UART port initialization */
void begin(const char * device, int baud, unsigned int timeout_ms, serial_parms_t * handler)
{
    handler->fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (handler->fd < 0) {
        printf("Error opening %s: %s\n", device, strerror(errno));
        handler->ret = -1;
        return;
    }
    /*baudrate 115200, 8 bits, no parity, 1 stop bit */
    set_interface_attribs(handler->fd, baud);
    /* Convert milli-seconds to deca-seconds */
    if (timeout_ms == 0) {
        /* pure non-blocking */
        set_mincount(handler->fd, 0, 0);                /* set to pure timed read */    
    }else if (timeout_ms < 100) {
        /* 100ms blocking */
        set_mincount(handler->fd, 0, 1);                /* set to pure timed read */    
    }else{
        /* specified timeout ms */
        set_mincount(handler->fd, 0, timeout_ms/100);   /* set to pure timed read */
    }
    handler->timeout = timeout_ms;
    return;
}


int available(serial_parms_t * input_handler)
{
    int bytes_avail;
    ioctl(input_handler->fd, FIONREAD, &bytes_avail);
    return bytes_avail;
}

int read_port(serial_parms_t * input_handler)
{
    return read(input_handler->fd, input_handler->buffer, 1);
}

void clear (serial_parms_t * input_handler)
{
    if(available(input_handler) > 0)
    {
        while(read_port(input_handler) > 0);
    }
}

int readBytesUntil(serial_parms_t * input_handler, char to_find, char * buffer, int max_size)
{   
    int cnt = 0;
    if(available(input_handler) > 0) {
        while (read_port(input_handler) > 0) {
            /* keep reading */
            buffer[cnt] = input_handler->buffer[0];
            if ((char) buffer[cnt] == to_find) {
                cnt++;
                return cnt;
            }else{
                cnt++;
                if (cnt >= max_size)
                    return max_size;
            }
        }
    }
    return 0;
}