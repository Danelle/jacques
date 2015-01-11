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
 * When JSocket writes data, it preppend the 4 bytes, it blocks to write
 * When JSocket reads data, it parses the first 4 bytes and reads the real data, in a non-blocking way 
 */
typedef struct _JSocket JSocket;

#define j_socket_data(jsock) ((jsock)->buf)->data
#define j_socket_data_length(jsock) ((jsock)->buf)->len

/*
 * Creates a new passive IPv4 socket, which listens on port
 * Returns NULL on error;
 */
JSocket *j_server_socket_new(gushort port, guint32 backlog);

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


/* wrappers for the raw syscalls */
int j_socket_write_raw(JSocket * jsock, const void *buf, guint32 count);
int j_socket_read_raw(JSocket * jsock, void *buf, guint32 count);
int j_socket_writev_raw(JSocket * jsock, const struct iovec *iov,
                        guint32 iovcnt);
int j_socket_readv_raw(JSocket * jsock, const struct iovec *iov,
                       guint32 iovcnt);
int j_socket_recv_raw(JSocket * jsock, void *buf, guint32 size,
                      gint32 flags);
int j_socket_accept_raw(JSocket * jsock, struct sockaddr *addr,
                        socklen_t * addrlen);

/*
 * Packages the data and write to the socket
 * Returns 1 if all data sent
 * Returns 0 on error
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


#endif                          /* __J_SOCKET_H__ */
