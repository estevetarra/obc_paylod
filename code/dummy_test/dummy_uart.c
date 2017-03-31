#include "simple_link.h"
#include "uart_handler.h"
#include "socket_utils.h"

static char dev_name[128];

void print_progress(float progress)
{
    /* From 0 to 1 is the progress marked */
    int barWidth = 70;
    int i;
    printf( "[");
    int pos = barWidth * progress;
    for (i = 0; i < barWidth; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %.0f%%\r", (progress * 100.0));
    fflush(stdout);
}

/* UART must be initialised anyway, independent of sockets or not */
/* The diagram is: UART always listening, server socket always listening */
/* The UART must start listening even if there is no Socket connected */
int main (int argc, char ** argv)
{
    /* open uart handler */
    int random_handler;
    serial_parms_t hserial;
    int ret;
    char buffer_stdin[128];
    simple_link_control_t control_sending;
    simple_link_control_t control_receiving;
    simple_link_packet_t packet_sending;
    simple_link_packet_t packet_receiving;
    int conf;
    if (argc > 1){
        if (argc == 3){
            strncpy(dev_name, argv[1], sizeof(dev_name));
            conf = atoi(argv[2]);
        }else{
            printf("WTF u doin\n");
            exit(1);
        }
    }else{
        printf("Introduce 1 argument\n");
     
       exit(1);
    }

    /* Waiting for something here */
    /* Send something to socket */
    /* The length will be the command request + command response (if any) */
    begin(dev_name, B115200, 0, &hserial);
    uint16_t correct_packets = 0;
    uint16_t total_packets = 0;
    uint16_t received_packets = 0;
    bool stop_sending = false;

    prepare_simple_link('J', 'F', 0, &control_sending);
    prepare_simple_link('J', 'F', 5, &control_receiving);

    srand(time(NULL));

    while ( stop_sending == false){
        strcpy(buffer_stdin, "TEST STRING");
        ret = strlen(buffer_stdin);
        set_simple_link_packet((uint8_t*) buffer_stdin, ret, 0, 0, &control_sending, &packet_sending);
        /* This sends 2 packets, first packet erroneous and second correct one */
        if (conf == 2){
            random_handler = rand()%4;
        }else{
            random_handler = rand()%2;
        }
        
        total_packets++;
        if (random_handler == 3){
            /* Change length to something bigger */
            packet_sending.fields.len = htons(rand()%65535);
            write(hserial.fd, &packet_sending, control_sending.full_size);
        }else if (random_handler == 1){
            /* Change 1 byte */
            packet_sending.fields.payload[0] ^= 0x4E;
            write(hserial.fd, &packet_sending, control_sending.full_size);
        }else if (random_handler == 2){
            /* Send less bytes */
            write(hserial.fd, &packet_sending, control_sending.full_size - 2);
        }else{
            correct_packets++;
            write(hserial.fd, &packet_sending, control_sending.full_size);
        }
        usleep(20 * 1000);
        while (available(&hserial) > 0){
            read_port(&hserial);
            if (get_simple_link_packet(hserial.buffer[0], &control_receiving, &packet_receiving) > 0){
                packet_receiving.fields.payload[packet_receiving.fields.len] = '\0';
                received_packets++;
                //printf("[UART]-> %d::%s from a total of: %d\n", packet.fields.len, packet.fields.payload, received_packets);
            }
        }
        print_progress((float) total_packets/1000.0);
        if (total_packets == 1000){
            stop_sending = true;
            printf("\n\n");
        }               
    }
    //}       
    if (stop_sending == true){
        /* wait for a while */
        usleep(500 * 1000);
        /* receive the last sent packets if any */
        while (available(&hserial) > 0){
            read_port(&hserial);
            if (get_simple_link_packet(hserial.buffer[0], &control_receiving, &packet_receiving) > 0){
                packet_receiving.fields.payload[packet_receiving.fields.len] = '\0';
                received_packets++;
                //printf("[UART]-> %d::%s from a total of: %d\n", packet.fields.len, packet.fields.payload, received_packets);
            }
        }            
        printf("Amount of received packets: %d from a correct send: %d and a total amount of sent: %d\n", received_packets, correct_packets, total_packets);
    }
    close(hserial.fd);
}