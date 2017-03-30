#include "socket_utils.h"
#include "uart_control.h"

int main(void)
{
    payload_command_handle_t cmd;
    int ret;
    int server = socket_init_local_client(53001);
    if (! (server > 0)){
        perror("Shit");
        exit(0);
    }
    while(1){
        if ( (ret = read_with_timeout(server, &cmd, sizeof(payload_command_handle_t), 100) ) > 0){
            printf("Command Request: %d\n", cmd.fields.command_request);
            strcpy(cmd.fields.command_response, "OK");
            cmd.fields.len = 1 + strlen(cmd.fields.command_response);
            printf("Generating Response...\n");
            write(server, &cmd, cmd.fields.len+1);
        }else if (ret == 0){   
            /* Legacy broh */
        }else{
            perror("End of socket (reading)");
            /* error */
            return 0;
        }
    }
}