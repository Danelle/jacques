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
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>


/* macros for read buffer */
#define j_socket_set_rdata_length(jsock, len) g_byte_array_set_size((jsock)->rbuf,(len))
#define j_socket_clear_rdata(jsock)  j_socket_set_rdata_length((jsock),0)
#define j_socket_append_rdata(jsock,data,len) g_byte_array_append((jsock)->rbuf,(data),(len))
#define j_socket_total_length(jsock) (jsock)->total_len
#define j_socket_set_total_length(jsock, len) (jsock)->total_len = (len)
#define j_socket_left_length(jsock) j_socket_total_length(jsock) - j_socket_data_length(jsock)

/* macros for write buffer */
#define j_socket_set_wdata_length(jsock, len) g_byte_array_set_size((jsock)->wbuf,(len))
#define j_socket_clear_wdata(jsock) j_socket_set_wdata_length((jsock),0)
#define j_socket_wdata_length(jsock)    (jsock)->wbuf->len
#define j_socket_wdata(jsock)   (jsock)->wbuf->data
#define j_socket_wdata_pop(jsock,len)   g_byte_array_remove_range((jsock)->wbuf,0,len)
#define j_socket_wdata_append(jsock,data,len)   g_byte_array_append((jsock)->wbuf,(data),(len))

/*
 * Creates a new JSocket from a native socket descriptor
 */
JSocket *j_socket_new_fromfd(int sockfd, struct sockaddr *addr,
                             socklen_t addrlen)
{
    JSocket *jsock = (JSocket *) g_slice_alloc(sizeof(JSocket));
    jsock->sockfd = sockfd;
    jsock->rbuf = g_byte_array_new();
    jsock->wbuf = g_byte_array_new();
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

    int set = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &set,
               sizeof(set));

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
 * Creates a new client IPv4 socket, connect to remote in blocking way
 * Returns NULL on error
 */
JSocket *j_client_socket_new(const gchar * remote, gushort port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return NULL;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(remote);
    if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
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
JSocket *j_socket_accept(JSocket * jsock)
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
    g_byte_array_free(jsock->rbuf, TRUE);
    g_byte_array_free(jsock->wbuf, TRUE);
    g_slice_free1(sizeof(JSocket), jsock);
}

int j_socket_accept_raw(JSocket * jsock, struct sockaddr *addr,
                        socklen_t * addrlen)
{
    int sockfd = j_socket_fd(jsock);
    int fd;
  AGAIN:
    errno = 0;
    fd = accept(sockfd, addr, addrlen);
    if (fd < 0) {
        if (errno == ENETDOWN || errno == EPROTO ||
            errno == ENOPROTOOPT || errno == EHOSTDOWN || errno == ENONET
            || errno == EHOSTUNREACH || errno == EOPNOTSUPP
            || errno == ENETUNREACH || errno == EAGAIN
            || errno == EWOULDBLOCK) {
            /* check `man 2 accept` #Error handling for details */
            goto AGAIN;
        }
    }
    return fd;
}

/*
 * Wrappers for system calls
 * recall if interrupted by signal
 */
int j_socket_write_raw(JSocket * jsock, const void *rbuf, guint32 count)
{
    int sockfd = j_socket_fd(jsock);
    int n;
  AGAIN:
    errno = 0;
    n = send(sockfd, rbuf, count, MSG_DONTWAIT);    /* Run! Don't wait for me! You're the hope of human */
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
    errno = 0;
    n = recv(sockfd, buf, count, MSG_DONTWAIT); /* Hold on! I'll be back! */
    if (n < 0 && errno == EINTR) {  /* interrupted by signal? */
        goto AGAIN;
    }
    return n;
}

/*
 * Packs up the buf and write to socket in non-blocking way
 * If all data is writen, return 1
 * If only part of data is writen, return 0, to be continue next time
 * If error occurs, return -1
 * 
 * Note, if j_socket_write() returns 0, then you can call it with NULL in buf next time, until all data is writen
 */
int j_socket_write(JSocket * jsock, const void *buf, guint32 count)
{
    guint32 size = j_socket_wdata_length(jsock);
    if (size == 0) {
        /* new data to write */
        if (buf == NULL) {
            return 1;           /* no data? must be a mistake */
        }
        gchar *len = pack_length4(count);
        j_socket_wdata_append(jsock, len, 4);
        j_socket_wdata_append(jsock, buf, count);
        g_free(len);
        size = j_socket_wdata_length(jsock);
    }

    gint n;
    count = size > 4096 ? 4096 : size;

    while (count > 0) {         /* writes segmentation */
        n = j_socket_write_raw(jsock, j_socket_wdata(jsock), count);
        if (n < 0) {
            if (errno == EAGAIN) {
                return 0;
            }
            return -1;          /* It's a real error */
        }
        j_socket_wdata_pop(jsock, n);
        size = j_socket_wdata_length(jsock);
        count = size > 4096 ? 4096 : size;
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
    j_socket_clear_rdata(jsock);
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
        int n = j_socket_read_raw(jsock, databuf, count);   /* in non-blocking way */
        if (n < 0) {
            if (errno == EAGAIN) {
                return 0;
            }
            return -1;
        } else if (n == 0) {
            return -1;
        }
        j_socket_append_rdata(jsock, databuf, n);
        left = j_socket_left_length(jsock);
        count = sizeof(databuf) > left ? left : sizeof(databuf);
    }
    if (left != 0) {            /* unknown error, will hanppen? */
        return -1;
    }
    j_socket_set_total_length(jsock, 0);    /* a whole package read successfully */
    return 1;
}
