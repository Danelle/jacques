/*
 * jpoll.h
 * Copyright (C) 2015 Wiky L <wiiiky@yeah.net>
 * 
 * Jacques is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Jacques is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __J_POLL_H__
#define __J_POLL_H__

#include "jsocket.h"
#include <sys/epoll.h>


typedef struct _JPoll JPoll;


#define J_POLL_EVENT_IN EPOLLIN
#define J_POLL_EVENT_OUT EPOLLOUT
#define J_POLL_EVENT_HUP EPOLLHUP
#define J_POLL_EVENT_ERR EPOLLERR


typedef struct {
    guint32 type;
    JSocket *jsock;
} JPollEvent;


/*
 * Creates a JPoll
 * Returns NULL on error
 */
JPoll *j_poll_new();


/*
 * Returns the list of ready JSocket since last j_poll_wait()
 * The return GList is maintained by JPoll, do not modify it,nor free it
 */
GList *j_poll_ready(JPoll * jp);


/*
 * Waits for events on JPoll instance. Up to maxevents 
 * When successfully, j_poll_wait() returns a number of ready JSocket.
 * then you can call j_poll_ready() to get the ready JSocket
 * or zero if no JSocket became ready during the request timeout milliseconds
 * When an error occurs, returns -1
 */
int j_poll_wait(JPoll * jp, guint maxevents, guint timeout);

/*
 * Registers the JSocket with event EPOLLIN
 * Returns 1 on success, otherwise 0
 */
int j_poll_addin(JPoll * jp, JSocket * jsock);

/*
 * Registers the JSocket with event EPOLLOUT
 * Returns 1 on success, otherwise 0
 */
int j_poll_addout(JPoll * jp, JSocket * jsock);


/*
 * Registers the JSocket with event EPOLLOUT|EPOLLIN
 * Returns 1 on success, otherwise 0
 */
int j_poll_addio(JPoll * jp, JSocket * jsock);

/*
 * Unregisters the JSocket
 * Returns 1 on success, otherwise 0
 */
int j_poll_delete(JPoll * jp, JSocket * jsock);



#endif                          /* __J_POLL_H__ */
