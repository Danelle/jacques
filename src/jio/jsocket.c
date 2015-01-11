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
#include "pack.h"
#include <glib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>


/*
 * Creates a new JSocket from a native socket descriptor
 */
JSocket *j_socket_new_fromfd(int sockfd)
{
    JSocket *jsock = (JSocket *) g_slice_alloc(sizeof(JSocket));
    j_socket_set_fd(jsock, sockfd);
    return jsock;
}

/*
 * Creates a new passive IPv4 socket, which listens on port
 * 
 * Returns NULL on error;
 */
JSocket *j_server_socket_new(unsigned short port, unsigned int backlog)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return NULL;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(sockfd);
        return NULL;
    }

    if (listen(sockfd, backlog) < 0) {
        close(sockfd);
        return NULL;
    }

    return j_socket_new_fromfd(sockfd);
}

/*
 * Closes the JSocket
 */
void j_socket_close(JSocket * jsock)
{
    close(j_socket_fd(jsock));
    g_slice_free1(sizeof(JSocket), jsock);
}


/*
 * Wrapper for write[v and read[v]
 * recall if interrupted by signal
 */
int j_socket_write(JSocket * jsock, const void *buf, unsigned int count)
{
    int sockfd = j_socket_fd(jsock);
    int n;
  AGAIN:
    n = write(sockfd, buf, count);
    if (n < 0 && errno == EINTR) {
        goto AGAIN;
    }
    return n;
}

int j_socket_read(JSocket * jsock, void *buf, unsigned int count)
{
    int sockfd = j_socket_fd(jsock);
    int n;
  AGAIN:
    n = read(sockfd, buf, count);
    if (n < 0 && errno == EINTR) {
        goto AGAIN;
    }
    return n;
}

int j_socket_writev(JSocket * jsock, const struct iovec *iov, int iovcnt)
{
    int sockfd = j_socket_fd(jsock);
    int n;
  AGAIN:
    n = writev(sockfd, iov, iovcnt);
    if (n < 0 && errno == EINTR) {
        goto AGAIN;
    }
    return n;
}

int j_socket_readv(JSocket * jsock, const struct iovec *iov, int iovcnt)
{
    int sockfd = j_socket_fd(jsock);
    int n;
  AGAIN:
    n = readv(sockfd, iov, iovcnt);
    if (n < 0 && errno == EINTR) {
        goto AGAIN;
    }
    return n;
}

/*
 * Packages the data and write to the socket
 * Returns 1 if all data sent
 * Returns 0 on error
 */
int j_socket_writeall(JSocket * jsock, const void *buf, unsigned int count)
{
    char *len = pack_length4(count);

    struct iovec iovs[2];
    iovs[0].iov_base = (void *) len;
    iovs[0].iov_len = 4;
    iovs[1].iov_base = (void *) buf;
    iovs[1].iov_len = count;

    int n = j_socket_writev(jsock, iovs, 2);
    free(len);

    if (n != count + 4) {
        return 0;
    }
    return 1;
}
