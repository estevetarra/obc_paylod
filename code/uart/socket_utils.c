//
//  socket_utils.c
//  3CAT-2_COMMS_SOCKET
//
//  Created by Juan Fran Muñoz Martin on 01/10/14.
//  Copyright (c) 2014 Juan Fran Muñoz Martin. All rights reserved.
//

#include "socket_utils.h"


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

int socket_init_local_client(int port)
{
    int fd;
    struct hostent *he;
    /* estructura que recibirá información sobre el nodo remoto */
    struct sockaddr_in server;
    /* información sobre la dirección del servidor */
    if ((he=gethostbyname("localhost"))==NULL){
        return -1;
    }
    
    if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *)he->h_addr);

    bzero(&(server.sin_zero), 8);
    
    if(connect(fd, (struct sockaddr *)&server,
               sizeof(struct sockaddr))==-1){
        return -1;
    }
    return fd;
}

int socket_new_client(int sockfd)
{
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    return accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
}

int socket_init_server(int port)
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        return -1;
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
        return -1;

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        return -1;

    listen(sockfd, 1);

    return sockfd;
}