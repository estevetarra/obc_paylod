#ifndef __UART_HANDLER_H__
#define __UART_HANDLER_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

#define UART_BUFFER		2048

typedef struct serial_parms_s{
	int 			fd;
	int 			ret;
	unsigned char 	buffer[UART_BUFFER];
	unsigned int 	timeout;
}serial_parms_t;

void begin(const char * device, int baud, unsigned int timeout_ms, serial_parms_t * handler);
int available(serial_parms_t * input_handler);
int read_port(serial_parms_t * input_handler);
void clear (serial_parms_t * input_handler);
int readBytesUntil(serial_parms_t * input_handler, char to_find, char * buffer, int max_size);

#endif

