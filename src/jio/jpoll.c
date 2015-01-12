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
#include <errno.h>


struct _JPoll {
    int epollfd;
    GList *jsocks;              /* the list of JSockets registered */
    GList *ready;
};

#define j_poll_add_jsocket(jp,jsock)    (jp)->jsocks=g_list_append((jp)->jsocks,(jsock))
#define j_poll_remove_jsocket(jp,jsock) (jp)->jsocks=g_list_remove((jp)->jsocks,(jsock))


#define j_poll_fd(jp)	(jp)->epollfd


GList *j_poll_ready(JPoll * jp)
{
    return jp->ready;
}

static inline void j_poll_clear_ready(JPoll * jp);

static inline JPollEvent *j_poll_event_new(struct epoll_event *event);
static inline void j_poll_event_free(JPollEvent * event);

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
    jp->ready = NULL;
    jp->jsocks = NULL;
    return jp;
}

/*
 * Waits for events on JPoll instance. Up to maxevents 
 * When successfully, j_poll_wait() returns a number of ready JSocket.
 * then you can call j_poll_ready() to get the ready JSocket
 * or zero if no JSocket became ready during the request timeout milliseconds
 * When an error occurs, returns -1
 */
int j_poll_wait(JPoll * jp, guint maxevents, guint timeout)
{
    struct epoll_event events[128];
    if (maxevents > 128) {
        maxevents = 128;
    } else if (maxevents == 0) {
        return 0;
    }
    j_poll_clear_ready(jp);
    int epollfd = j_poll_fd(jp);
    int n;
  AGAIN:
    n = epoll_wait(epollfd, events, maxevents, timeout);
    if (n < 0) {
        if (errno == EINTR) {
            goto AGAIN;
        }
        return -1;
    } else if (n == 0) {
        return 0;
    }

    int i;
    for (i = 0; i < n; i++) {
        jp->ready = g_list_append(jp->ready, j_poll_event_new(&events[i]));
    }
    return n;
}


static inline int j_poll_add_internal(JPoll * jp, JSocket * jsock,
                                      guint32 events)
{
    int epollfd = j_poll_fd(jp);
    int sockfd = j_socket_fd(jsock);

    struct epoll_event event;
    event.events = events;      /* epoll_wait will always wait for EPOLLERR and EPOLLHUP, not necessary to set here */
    event.data.ptr = (void *) jsock;

    j_poll_add_jsocket(jp, jsock);

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

    j_poll_remove_jsocket(jp, jsock);

    struct epoll_event event;   /* linux 2.6.9 required a non-null pointer in event */
    return !epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, &event);
}

static inline void j_poll_clear_ready(JPoll * jp)
{
    g_list_free_full(jp->ready, (GDestroyNotify) j_poll_event_free);
    jp->ready = NULL;
}

static inline JPollEvent *j_poll_event_new(struct epoll_event *event)
{
    JPollEvent *jpe = (JPollEvent *) g_slice_alloc(sizeof(JPollEvent));
    jpe->type = event->events;
    jpe->jsock = (JSocket *) event->data.ptr;
}

static inline void j_poll_event_free(JPollEvent * jpe)
{
    g_slice_free1(sizeof(JPollEvent), jpe);
}
