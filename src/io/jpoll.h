/*
 * jpoll.h
 * Copyright (C) 2015 Wiky L <wiiiky@outlook.com>
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


/* this structure is public */
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
 * Gets all the JSockets that is registered in JPoll
 * The return GList is maintained by JPoll
 */
GList *j_poll_all(JPoll * jp);


/*
 * Gets the count of JSockets
 */
gint j_poll_count(JPoll * jp);


/*
 * Waits for events on JPoll instance. Up to maxevents
 * When successfully, j_poll_wait() returns a number of ready JSocket.
 * then you can call j_poll_ready() to get the ready JSocket
 * or zero if no JSocket became ready during the request timeout milliseconds
 * When an error occurs, returns -1
 */
gint j_poll_wait(JPoll * jp, JPollEvent * jevents, guint maxevents,
                 gint timeout);


/*
 * Registers a JSocket
 * Returns 1 on success, otherwise 0
 */
gint j_poll_register(JPoll * jp, JSocket * jsock, guint32 types);


/*
 * Modify the event associated to the JSocket
 * Returns 1 on success, otherwise 0
 */
gint j_poll_modify(JPoll * jp, JSocket * jsock, guint32 type);

/*
 * Unregisters the JSocket
 * Returns 1 on success, otherwise 0
 */
gint j_poll_delete(JPoll * jp, JSocket * jsock);


/*
 * Unregisters the Jsocket and close it
 */
gint j_poll_delete_close(JPoll * jp, JSocket * jsock);


/*
 * Closes JPoll
 * This function will close JPoll and free all the memory used by JPoll
 * But not free JSockets registered.
 * So get all JSockets registered before close the JPoll
 */
gint j_poll_close(JPoll * jp);

/*
 * Closes JPoll and all JSocket registered
 */
gint j_poll_close_all(JPoll * jp);



/*
 * Removes all JSockets that are not active during last timeout seconds
 * Returns the count of JSockets that are removed
 */
guint32 j_poll_remove_timeout(JPoll * jp, guint64 timeout);



#endif                          /* __J_POLL_H__ */
