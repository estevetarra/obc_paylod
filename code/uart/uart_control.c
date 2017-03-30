/* system includes here */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>

/* user addition includes here */
#include "uart_control.h"

/* shared circular buffer */
static circ_buff_t queue;
/* shared mutex */
pthread_mutex_t queue_mutex;

int read_with_timeout(int fd, void * p, size_t size, unsigned long ms)
{
    // timeout structure passed into select
    struct timeval tv;
    // fd_set passed into select
    fd_set fds;
    int control_ret, read_ret;
    // Set up the timeout.  here we can wait for 1 second
    if (ms >= 1000){
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
    }else{
        tv.tv_sec = 0;
        tv.tv_usec = ms * 1000;
    }

    // Zero out the fd_set - make sure it's pristine
    FD_ZERO(&fds);
    // Set the FD that we want to read
    FD_SET(fd, &fds);
    // select takes the last file descriptor value + 1 in the fdset to check,
    // the fdset for reads, writes, and errors.  We are only passing in reads.
    // the last parameter is the timeout.  select will return if an FD is ready or 
    // the timeout has occurred
    if ( (control_ret = select(fd+1, &fds, NULL, NULL, &tv) ) == -1){
        return -1;
    }
    // return 0 if fd is not ready to be read.
    if ( ( control_ret = FD_ISSET(fd, &fds) ) > 0 ){
        /* Something to read! */
        read_ret = read(fd, p, size);
        if (read_ret == 0){
            return -1;
        }else{
            return read_ret;
        }
    }else{
        /* maybe is a -1 */
        return control_ret;
    }
}

bool _safe_dequeue(void * queue, void * val)
{
    bool ret;
    pthread_mutex_lock(&queue_mutex);
    ret = dequeue(queue, val);
    pthread_mutex_unlock(&queue_mutex);
    return ret;
}

bool _safe_enqueue(void * queue, void * val)
{
    bool ret;
    pthread_mutex_lock(&queue_mutex);  
    ret = enqueue(queue, val);
    pthread_mutex_unlock(&queue_mutex);
    return ret;
}

void * socket_work(void * args)
{
    uint8_t buffer[256];
    command_handle_t cmd;
    int server = socket_init_server(53001);
    int client;
    int ret;
    bool socket_connected = false;
    /* FD timeout reading */
    while(1){
        if (!socket_connected){
            client = socket_new_client(server);
            socket_connected = true;
        }else{
            if ( (ret = read_with_timeout(client, buffer, sizeof(buffer), 100) ) > 0){
                buffer[ret] = '\0';
                printf("Received message: %s", buffer);
            }else if (ret == 0){
                /* There is something on the queue? */
                if (_safe_dequeue(&queue, &cmd)){
                    if (write(client, cmd.fields.command, cmd.fields.len) <= 0){
                        perror("End of socket (writing)");
                        socket_connected = false;
                    }
                }
                /* Legacy broh */
            }else{
                perror("End of socket (reading)");
                socket_connected = false;
                /* error */
            }
        }
    }
}

void * uart_work(void * args)
{
    command_handle_t cmd;
    /* Waiting for something here */
    /* Send something to socket */
    strcpy(cmd.fields.command, "HOLA!");
    cmd.fields.len = strlen(cmd.fields.command);
    while(1){
        _safe_enqueue(&queue, &cmd);
        sleep(5);
    }
}

/* UART must be initialised anyway, independent of sockets or not */
/* The diagram is: UART always listening, server socket always listening */
/* The UART must start listening even if there is no Socket connected */
int main (int argc, char ** argv)
{
    /* this creates a tcp server socket */
    pthread_t socket_thread;
    pthread_t uart_thread;
    /* Element size? */
    /* 10 commands? */
    queue_init(&queue, sizeof(command_handle_t), 10);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_create(&socket_thread, NULL, socket_work, NULL);
    pthread_create(&uart_thread, NULL, uart_work, NULL);

    while(1)
    {
        sleep(1);
    }

}