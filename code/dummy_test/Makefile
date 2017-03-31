CC=gcc
CFLAGS=-I. -I../common/ -Wall
LFLAGS=-lpthread

OBJ_DPAY = dummy_payload_manager.o ../common/socket_utils.o 
OBJ_DUART = dummy_uart.o ../common/uart_handler.o ../common/simple_link.o ../common/socket_utils.o

.PHONY: all dummy_uart dummy_payload clean

all: dummy_uart dummy_payload clean

dummy_uart: $(OBJ_DUART)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)
  
dummy_payload: $(OBJ_DPAY)
	gcc -o $@ $^ $(CFLAGS) $(LFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o *~
	rm -f ../common/*.o
