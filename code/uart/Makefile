CC=gcc
CFLAGS=-I. -I../common/ -Wall
LFLAGS=-lpthread

OBJ_UART = uart_control.o ../common/uart_handler.o ../common/socket_utils.o ../common/circular_queue.o ../common/simple_link.o

.PHONY: all uart_control clean

all: uart_control clean

uart_control: $(OBJ_UART)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o *~
	rm -f ../common/*.o