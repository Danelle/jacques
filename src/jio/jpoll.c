/*
 * jpoll.c
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

#include "jpoll.h"
#include <glib.h>
#include <sys/epoll.h>


struct _JPoll {
    int epollfd;
};


#define j_poll_fd(jp)	(jp)->epollfd


/* Creates a JPoll from a existing file descriptor */
static inline JPoll *j_poll_new_fromfd(int fd);

/*
 * Creates a JPoll
 * Returns NULL on error
 */
JPoll *j_poll_new()
{
    int fd = epoll_create(1);   /* size must be greater than zero, but not used */
    if (fd < 0) {
        return NULL;
    }
    return j_poll_new_fromfd(fd);
}


static inline JPoll *j_poll_new_fromfd(int fd)
{
    JPoll *jp = (JPoll *) g_slice_alloc(sizeof(JPoll));
    jp->epollfd = fd;
    return jp;
}


static inline int j_poll_add_internal(JPoll * jp, JSocket * jsock,
                                      guint32 events)
{
    int epollfd = j_poll_fd(jp);
    int sockfd = j_socket_fd(jsock);

    struct epoll_event event;
    event.events = events;      /* epoll_wait will always wait for EPOLLERR and EPOLLHUP, not necessary to set here */
    event.data.ptr = (void *) jsock;

    return !epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
}

/*
 * Registers the JSocket
 * Returns 1 on success, otherwise 0
 */
int j_poll_addin(JPoll * jp, JSocket * jsock)
{
    return j_poll_add_internal(jp, jsock, EPOLLIN);
}

/*
 * Registers the JSocket with event EPOLLOUT
 * Returns 1 on success, otherwise 0
 */
int j_poll_addout(JPoll * jp, JSocket * jsock)
{
    return j_poll_add_internal(jp, jsock, EPOLLOUT);
}

/*
 * Registers the JSocket with event EPOLLOUT|EPOLLIN
 * Returns 1 on success, otherwise 0
 */
int j_poll_addio(JPoll * jp, JSocket * jsock)
{
    return j_poll_add_internal(jp, jsock, EPOLLIN | EPOLLOUT);
}

/*
 * Unregisters the JSocket
 * Returns 1 on success, otherwise 0
 */
int j_poll_delete(JPoll * jp, JSocket * jsock)
{
    int epollfd = j_poll_fd(jp);
    int sockfd = j_socket_fd(jsock);

    struct epoll_event event;   /* linux 2.6.9 required a non-null pointer in event */
    return !epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, &event);
}
