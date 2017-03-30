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
    payload_command_handle_t cmd;
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
            if ( (ret = read_with_timeout(client, &cmd, sizeof(payload_command_handle_t), 100) ) > 0){
                printf("Command Requested: %d\n", cmd.fields.command_request);
                printf("Command Length: %d\n", cmd.fields.len);
                cmd.fields.command_response[cmd.fields.len - 1] = '\0';
                printf("Received Response: %s\n", cmd.fields.command_response);
            }else if (ret == 0){
                /* There is something on the queue? */
                if (_safe_dequeue(&queue, &cmd)){
                    if (write(client, &cmd, cmd.fields.len+1) <= 0){
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
    payload_command_handle_t cmd;
    /* Waiting for something here */
    /* Send something to socket */
    cmd.fields.command_request = 0;
    /* The length will be the command request + command response (if any) */
    cmd.fields.len = 1;
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
    queue_init(&queue, sizeof(payload_command_handle_t), 10);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_create(&socket_thread, NULL, socket_work, NULL);
    pthread_create(&uart_thread, NULL, uart_work, NULL);

    while(1)
    {
        sleep(1);
    }

}