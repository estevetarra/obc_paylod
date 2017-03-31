/* system includes here */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>

/* user addition includes here */
#include "simple_link.h"
#include "uart_control.h"
#include "uart_handler.h"
/* shared circular buffer */
static circ_buff_t queue;
/* shared mutex */
static pthread_mutex_t queue_mutex;

static char dev_name[128];

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
    payload_command_handle_t cmd;
    int client;
    int ret;
    bool socket_connected = false;
    int server = socket_init_server(53001);
    if (server <= 0){
        perror("Server faield to init");
        exit(1);
    }    
    /* FD timeout reading */
    while(1){
        if (!socket_connected){
            client = socket_new_client(server);            
            if (client > 0){
                socket_connected = true;
            }else if (client == 0){
                /* still wait */
                socket_connected = false;
            }else{
                perror("Client failed to connect");
                exit(1);
            }
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
    //payload_command_handle_t cmd;
    /* open uart handler */
    serial_parms_t hserial;
    /* Waiting for something here */
    /* Send something to socket */
    //cmd.fields.command_request = 0;
    /* The length will be the command request + command response (if any) */
    begin(dev_name, B115200, 100, &hserial);
    //  cmd.fields.len = 1;

    simple_link_control_t control_sending;
    simple_link_packet_t packet_sending;
    simple_link_control_t control_receiving;
    simple_link_packet_t packet_receiving;

    char buffer[128];

    prepare_simple_link('J', 'F', 0, &control_sending);
    prepare_simple_link('J', 'F', 5, &control_receiving);
    int ret;
    uint16_t received = 0;
    while(1){
        while (available(&hserial) > 0){
            read_port(&hserial);
            if ( (ret = get_simple_link_packet(hserial.buffer[0], &control_receiving, &packet_receiving) ) > 0){
                packet_receiving.fields.payload[packet_receiving.fields.len] = '\0';
                //printf("[UART]-> %d::%s\n", packet.fields.len, packet.fields.payload);
                strcpy(buffer, "Response!");
                set_simple_link_packet((uint8_t*) buffer, strlen(buffer)+1, 0, 0, &control_sending, &packet_sending);
                write(hserial.fd, &packet_sending, control_sending.full_size);
                received++;
                //printf("received: %d\n", received);
            }else if (ret < 0){
                //printf("Ret error: %d\n", ret);
            }
        }
        //_safe_enqueue(&queue, &cmd);
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
    if (argc > 1){
        if (argc == 2){
            strncpy(dev_name, argv[1], sizeof(dev_name));
        }else{
            printf("WTF u doin\n");
            exit(1);
        }
    }else{
        printf("Introduce 1 argument\n");
        exit(1);
    }
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