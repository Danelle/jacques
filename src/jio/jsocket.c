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


/* this structure is private */
struct _JSocket {
    int sockfd;                 /* native socket descriptor */
    GByteArray *buf;            /* read buffer */
    guint32 total_len;          /* the total size of the whole package */
    struct sockaddr_storage addr;
    socklen_t addrlen;
};


/* private macros */
#define j_socket_fd(jsock) (jsock)->sockfd
#define j_socket_set_data_length(jsock, len) g_byte_array_set_size((jsock)->buf,(len))
#define j_socket_clear_data(jsock)  j_socket_set_data_length((jsock),0)
#define j_socket_append_data(jsock,data,len) g_byte_array_append((jsock)->buf,(data),(len))
#define j_socket_total_length(jsock) (jsock)->total_len
#define j_socket_set_total_length(jsock, len) (jsock)->total_len = (len)
#define j_socket_left_length(jsock) j_socket_total_length(jsock) - j_socket_data_length(jsock)

/*
 * Creates a new JSocket from a native socket descriptor
 */
JSocket *j_socket_new_fromfd(int sockfd, struct sockaddr *addr,
                             socklen_t addrlen)
{
    JSocket *jsock = (JSocket *) g_slice_alloc(sizeof(JSocket));
    jsock->sockfd = sockfd;
    jsock->buf = g_byte_array_new();
    jsock->total_len = 0;

    if (addr) {
        memcpy(&(jsock->addr), addr, addrlen);
    } else {
        memset(&(jsock->addr), 0, sizeof(jsock->addr));
    }

    return jsock;
}

/*
 * Creates a new passive IPv4 socket, which listens on port
 * 
 * Returns NULL on error;
 */
JSocket *j_server_socket_new(gushort port, guint32 backlog)
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

    return j_socket_new_fromfd(sockfd, (struct sockaddr *) &addr,
                               sizeof(addr));
}

/*
 * Accepts a connection and construct a new JSocket, will block
 * Returns NULL on error
 */
JSocket *j_server_socket_accept(JSocket * jsock)
{
    struct sockaddr_storage addr;
    socklen_t addrlen;

    int fd =
        j_socket_accept_raw(jsock, (struct sockaddr *) &addr, &addrlen);
    if (fd < 0) {
        return NULL;
    }
    return j_socket_new_fromfd(fd, (struct sockaddr *) &addr, addrlen);
}

/*
 * Closes the JSocket
 */
void j_socket_close(JSocket * jsock)
{
    close(j_socket_fd(jsock));
    g_byte_array_free(jsock->buf, TRUE);
    g_slice_free1(sizeof(JSocket), jsock);
}

int j_socket_accept_raw(JSocket * jsock, struct sockaddr *addr,
                        socklen_t * addrlen)
{
    int sockfd = j_socket_fd(jsock);
    int fd;
  AGAIN:
    fd = accept(sockfd, addr, addrlen);
    if (fd < 0 && errno == EINTR) {
        goto AGAIN;
    }
    return fd;
}

/*
 * Wrapper for write[v and read[v]
 * recall if interrupted by signal
 */
int j_socket_write_raw(JSocket * jsock, const void *buf, guint32 count)
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

int j_socket_read_raw(JSocket * jsock, void *buf, guint32 count)
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

int j_socket_writev_raw(JSocket * jsock, const struct iovec *iov,
                        guint32 iovcnt)
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

int j_socket_readv_raw(JSocket * jsock, const struct iovec *iov,
                       guint32 iovcnt)
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

int j_socket_recv_raw(JSocket * jsock, void *buf, guint32 size,
                      gint32 flags)
{
    int sockfd = j_socket_fd(jsock);
    int n;
  AGAIN:
    n = recv(sockfd, buf, size, flags);
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
int j_socket_write(JSocket * jsock, const void *buf, guint32 count)
{
    gchar *len = pack_length4(count);

    struct iovec iovs[2];
    iovs[0].iov_base = (void *) len;
    iovs[0].iov_len = 4;
    iovs[1].iov_base = (void *) buf;
    iovs[1].iov_len = count;

    int n = j_socket_writev_raw(jsock, iovs, 2);
    free(len);

    if (n != count + 4) {
        return 0;
    }
    return 1;
}


/*
 * Reads the length of a package
 * Returns 1 if success, 0 otherwise
 */
static int j_socket_read_length(JSocket * jsock)
{
    gchar lenbuf[4];
    int n = j_socket_read_raw(jsock, lenbuf, 4);
    if (n != 4) {               /* error */
        return 0;
    }
    guint32 length = unpack_length4(lenbuf);
    if (length == 0) {
        return 0;
    }
    j_socket_set_total_length(jsock, length);
    j_socket_clear_data(jsock);
    return 1;
}

/*
 * Reads a whole package
 * Returns 0 if not all data recevied (should continue next time)
 * Returns 1 if all data recevied
 * Returns -1 if error occurs
 *
 * After a successful read (a whole package data recevied), 
 * call j_socket_data() to get the data
 * call j_socket_data_length() to get the data length
 */
int j_socket_read(JSocket * jsock)
{
    if (j_socket_total_length(jsock) == 0 && j_socket_read_length(jsock) == 0) {    /* read the length of a new package */
        /* error */
        return -1;
    }

    gchar databuf[4096];
    gint32 left = j_socket_left_length(jsock);
    guint32 count = sizeof(databuf) > left ? left : sizeof(databuf);
    while (left > 0) {
        int n = j_socket_recv_raw(jsock, databuf, count, MSG_DONTWAIT); /* non-blocking reading */
        if (n < 0) {
            if (errno == EAGAIN) {
                return 0;
            }
            return -1;
        }
        j_socket_append_data(jsock, databuf, n);
        left = j_socket_left_length(jsock);
        count = sizeof(databuf) > left ? left : sizeof(databuf);
    }
    if (left != 0) {            /* unknown error, will hanppen? */
        return -1;
    }
    j_socket_set_total_length(jsock, 0);    /* a whole package read successfully */
    return 1;
}
