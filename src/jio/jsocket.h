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


/*
 * JSocket - socket that receives/sends packaged data
 */


typedef struct{
     int sockfd;
}JSocket;

#define j_socket_fd(jsock) (jsock)->sockfd
#define j_socket_set_fd(jsock,fd) (jsock)->sockfd = (fd)

/*
 * Creates a new passive IPv4 socket, which listens on port
 * Returns NULL on error;
 */
JSocket *j_server_socket_new(unsigned short port,unsigned int backlog);


/*
 * Creates a new JSocket from a native socket descriptor
 */
JSocket *j_socket_new_fromfd(int sockfd);

/*
 * Closes the JSocket
 */
void j_socket_close(JSocket *jsock);


/* wrapper for write and read */
int j_socket_write(JSocket *jsock,const void *buf, unsigned int count);
int j_socket_read(JSocket *jsock, void *buf, unsigned int count);

/*
 * Packages the data and write to the socket
 * Returns 1 if all data sent
 * Returns 0 on error
 */
int j_socket_writeall(JSocket *jsock, const void *buf, unsigned int count);



#endif /* __J_SOCKET_H__ */