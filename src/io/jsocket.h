/*
 * jsocket.h
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

#ifndef __J_SOCKET_H__
#define __J_SOCKET_H__

#include <sys/uio.h>
#include <netinet/in.h>
#include <glib.h>

/*
 * JSocket - IPv4 stream socket that receives/sends packaged data
 * 
 * Every package starts with 4 bytes which means the length of rest data
 * When JSocket writes data, it preppend the 4 bytes, 
 * When JSocket reads data, it parses the first 4 bytes and reads the real data,
 * in a non-blocking way 
 */
typedef struct _JSocket JSocket;

struct _JSocket {
    int sockfd;                 /* native socket descriptor */

    /* read buffer */
    GByteArray *rbuf;
    guint32 total_len;          /* the total size of the whole package */

    /* write buffer */
    GByteArray *wbuf;

    struct sockaddr_storage addr;
    socklen_t addrlen;

    guint64 active;             /* the timestamp of last action */


    /* extend data */
    gint64 data1;
    gpointer data2;
};
/* use macros to access the members */

#define j_socket_fd(jsock) (jsock)->sockfd
/* get read buffer data & length */
#define j_socket_data(jsock) ((void*)((jsock)->rbuf)->data)
#define j_socket_data_length(jsock) (((jsock)->rbuf)->len)


#define j_socket_set_extend_data1(jsock,data)   ((jsock)->data1=data)
#define J_socket_set_extend_data2(jsock,data)   ((jsock)->data2=data)
#define j_socket_get_extend_data1(jsock)        ((jsock)->data1)
#define j_socket_get_extend_data2(jsock)        ((jsock)->data2)


/* get the timestamp of JSocket last action */
#define j_socket_active_time(jsock) ((jsock)->active)

/*
 * Creates a new passive IPv4 socket, which listens on port
 * Returns NULL on error;
 */
JSocket *j_server_socket_new(gushort port, guint32 backlog);

/*
 * Creates a new client IPv4 socket, connect to remote in blocking way
 * Returns NULL on error
 */
JSocket *j_client_socket_new(const gchar * remote, gushort port);

/*
 * Creates a new JSocket from a native socket descriptor
 * @param addr must be type of struct sockaddr_in
 */
JSocket *j_socket_new_fromfd(int sockfd, struct sockaddr *addr,
                             socklen_t addrlen);

/*
 * Accepts a connection and construct a new JSocket, will block
 * Returns NULL on error
 */
JSocket *j_socket_accept(JSocket * jsock);

/*
 * Closes the JSocket
 */
void j_socket_close(JSocket * jsock);


/* wrappers for syscalls, non-blocking */
int j_socket_write_raw(JSocket * jsock, const void *buf, guint32 count);
int j_socket_read_raw(JSocket * jsock, void *buf, guint32 count);
/* blocking */
int j_socket_accept_raw(JSocket * jsock, struct sockaddr *addr,
                        socklen_t * addrlen);

/*
 * Packs up the buf and write to socket in non-blocking way
 * If all data is writen, return 1
 * If only part of data is writen, return 0, should continue next time
 * If error occurs, return -1
 * 
 * Note, if j_socket_write() returns 0, then you can it with buf NULL next time, until all data is writen
 */
int j_socket_write(JSocket * jsock, const void *buf, guint32 count);

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
int j_socket_read(JSocket * jsock);

/*
 * Gets the socket address
 */
const gchar *j_socket_address(JSocket * jsock);


#endif                          /* __J_SOCKET_H__ */
