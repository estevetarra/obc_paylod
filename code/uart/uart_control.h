#ifndef __UART_CONTROL_H__
#define __UART_CONTROL_H__

#include <stdint.h>

#include "simple_link.h"
#include "socket_utils.h"
#include "circular_queue.h"

#define MAX_COMMAND_SIZE 64

typedef union __attribute__ ((__packed__)) command_handle_u{
	uint8_t 	raw[MAX_COMMAND_SIZE + 1];
	struct __attribute__ ((__packed__)){
		uint8_t 	len;
		uint8_t 	command[MAX_COMMAND_SIZE];
	}fields;
}command_handle_t;

#endif
