/* system includes here */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>

/* user addition includes here */
#include "simple_link.h"
#include "uart_control.h"
#include "uart_handler.h"
#include "command_definition.h"

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

/* Uses CMD to make an answer, filling a packet with Control settings */
int process_command(serial_parms_t * s, command_def_t * cmd, simple_link_control_t * c)
{
    file_command_t file;
    command_def_t answer;
    simple_link_packet_t packet;
    int ret;
    if (cmd == NULL){
        return -1;
    }
    printf("Received command: %d at time %u\n", cmd->fields.command_id, cmd->fields.timestamp);
    /* We already know cmd length by means of cmd->fields.len + CD_HEADER_SIZE */ 
    switch (cmd->fields.command_id){
        case CD_HELLO: 
            printf("HELLO command received, Hello is returned\n");
            answer.fields.timestamp = time(NULL);
            answer.fields.command_id = CD_HELLO;
            answer.fields.len = 0;
            ret = set_simple_link_packet(&answer, answer.fields.len + CD_HEADER_SIZE, 0, 0, c, &packet);
            if (ret > 0){
                write(s->fd, &packet, ret);
            }
        break;

        case CD_START:
            printf("START command received, ACK is returned\n");

        break;

        case CD_STOP:
            printf("STOP command received, ACK is returned\n");

        break;

        case CD_STATUS:
            printf("STATUS command received, STATUS is returned\n");

        break;

        case CD_SET:
            printf("SET command received, ACK is returned\n");

        break;

        case CD_GET:
            printf("GET command received, File is returned\n");
            /* 256 first bytes indicate the absolute path to get the file from */
            /* This program goes there, if tar.gz the path and sends it */
            printf("");
        break;

        case CD_SET_SAT_TLE:
            printf("SET command received, ACK is returned\n");

        break;

        case CD_SET_GPS_TLE:
            printf("SET command received, ACK is returned\n");

        break;

        case CD_SET_GAL_TLE:
            printf("SET command received, ACK is returned\n");

        break;

        case CD_SET_MANAGER_CONF:
            printf("SET command received, ACK is returned\n");

        break;

        case CD_SET_GNSS_CONF:
            printf("SET command received, ACK is returned\n");

        break;

        case CD_GET_GNSS:
            printf("GET command received, File is returned\n");

        break;

        case CD_GET_RAD:
            printf("GET command received, File is returned\n");

        break;

        case CD_GET_AIS:
            printf("GET command received, File is returned\n");

        break;

        default:
        break;
    }
    return 0;
}

void * uart_work(void * args)
{
    //payload_command_handle_t pay_cmd;
    command_def_t uart_cmd;

    serial_parms_t hserial;
    int ret;

    simple_link_control_t   control_sending;
    simple_link_packet_t    packet_sending;

    simple_link_control_t   control_receiving;
    simple_link_packet_t    packet_receiving;

    begin(dev_name, B115200, 100, &hserial);

    prepare_simple_link('J', 'F', 0, &control_sending);
    prepare_simple_link('J', 'F', 5, &control_receiving);

    while(1){
        while (available(&hserial) > 0){
            read_port(&hserial);
            if ( (ret = get_simple_link_packet(hserial.buffer[0], &control_receiving, &packet_receiving) ) > 0){
                memcpy(&uart_cmd, &packet_receiving.fields.payload, packet_receiving.fields.len);
                process_command(&hserial, &uart_cmd, &control_sending);
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