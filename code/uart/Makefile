CC=gcc
CFLAGS=-I.
LFLAGS=-lpthread

DEPS = hellomake.h

OBJ_UART = uart_control.o socket_utils.o circular_queue.o
OBJ_PAY = dummy_payload_manager.o socket_utils.o 

.PHONY: all uart_control dummy_payload clean

all: uart_control dummy_payload clean

uart_control: $(OBJ_UART)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)

dummy_payload: $(OBJ_PAY)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o *~