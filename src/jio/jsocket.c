/*
 * jsocket.c
 *
 * Copyright (C) 2015 - Wiky L <wiiiky@outlook.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "jsocket.h"
#include <glib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


/*
 * Creates a new JSocket from a native socket descriptor
 */
JSocket *j_socket_new_fromfd(int sockfd)
{
    JSocket *jsock = (JSocket*)g_slice_alloc(sizeof(JSocket));
    j_socket_set_fd(jsock,sockfd);
    return jsock;
}

/*
 * Creates a new passive IPv4 socket, which listens on port
 * 
 * Returns NULL on error;
 */
JSocket *j_server_socket_new(unsigned short port, unsigned int backlog)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        return NULL;
    }
    struct sockaddr_in addr;
    memset(&addr, 0 ,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sockfd, (struct sockaddr*)&addr,sizeof(addr))<0){
        close(sockfd);
        return NULL;
    }

    if(listen(sockfd,backlog)<0){
        close(sockfd);
        return NULL;
    }

    return j_socket_new_fromfd(sockfd);
}

/*
 * Closes the JSocket
 */
void j_socket_close(JSocket *jsock)
{
    close(j_socket_fd(jsock));
    g_slice_free1(sizeof(JSocket), jsock);
}


/*
 * Wrapper for write() and read()
 * recall if interrupted by signal
 */
int j_socket_write(JSocket *jsock,const void *buf, unsigned int count)
{
    int sockfd = j_socket_fd(jsock);
    int n;
    AGAIN:
    n = write(sockfd, buf, count);
    if(n<0 && errno ==EINTR){
        goto AGAIN;
    }
    return n;
}

int j_socket_read(JSocket *jsock, void *buf, unsigned int count)
{
    int sockfd = j_socket_fd(jsock);
    int n;
    AGAIN:
    n = read(sockfd, buf, count);
    if(n<0 && errno ==EINTR){
        goto AGAIN;
    }
    return n;
}

/*
 * Packages the data and write to the socket
 * Returns 1 if all data sent
 * Returns 0 on error
 */
int j_socket_writeall(JSocket *jsock, const void *buf, unsigned int count)
{
    int sockfd = j_socket_fd(jsock);
    int n=0;

    while(count>0){
        n = j_socket_write(jsock, buf + n, count);
        if(n<0){    /* on error */
            return 0;
        }
        count -= n;
    }
    return 1;
}