#include "socket_utils.h"
#include "uart_control.h"

int main(void)
{
    payload_command_handle_t cmd;
    int ret;
    int server;
    bool socket_connected = false;
    while(1){
        if (!socket_connected){
            server = socket_init_local_client(53001);
            if (server > 0){
                socket_connected = true;
            }else{
                perror("Client failed to connect");
                socket_connected = false;
                sleep(1);
            }
        }else{
            if ( (ret = read_with_timeout(server, &cmd, sizeof(payload_command_handle_t), 100) ) > 0){
                printf("Command Request: %d\n", cmd.fields.command_request);
                strcpy(cmd.fields.command_response, "OK");
                cmd.fields.len = 1 + strlen(cmd.fields.command_response);
                printf("Generating Response...\n");
                if (write(server, &cmd, cmd.fields.len+1) <= 0){
                    perror("End of socket (writing)");
                    socket_connected = false;
                }
            }else if (ret == 0){   
                /* Legacy broh */
            }else{
                perror("End of socket (reading)");
                socket_connected = false;
                /* error */
            }            
        }
    }
}