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
#include <time.h>
#include <glib.h>
#include <errno.h>


struct _JPoll {
    gint epollfd;
    GList *jsocks;              /* the list of JSockets registered */
    gint count;                 /* the length of jsocks */
};


static inline void j_poll_add_jsocket(JPoll * jp, JSocket * jsock)
{
    jp->jsocks = g_list_append(jp->jsocks, jsock);
    jp->count++;
}

static inline void j_poll_remove_jsocket(JPoll * jp, JSocket * jsock)
{
    jp->jsocks = g_list_remove(jp->jsocks, jsock);
    jp->count--;
}


#define j_poll_fd(jp)	(jp)->epollfd


/*
 * Gets all the JSockets that is registered in JPoll
 */
GList *j_poll_all(JPoll * jp)
{
    return jp->jsocks;
}

/*
 * Gets the count of JSockets
 */
gint j_poll_count(JPoll * jp)
{
    return jp->count;
}


static inline JPollEvent *j_poll_event_new(struct epoll_event *event);
static inline void j_poll_event_free(JPollEvent * event);

/* Creates a JPoll from a existing file descriptor */
static inline JPoll *j_poll_new_fromfd(gint fd);

/*
 * Creates a JPoll
 * Returns NULL on error
 */
JPoll *j_poll_new()
{
    gint fd = epoll_create(1);  /* size must be greater than zero, but not used */
    if (fd < 0) {
        return NULL;
    }
    return j_poll_new_fromfd(fd);
}


static inline JPoll *j_poll_new_fromfd(gint fd)
{
    JPoll *jp = (JPoll *) g_slice_alloc(sizeof(JPoll));
    jp->epollfd = fd;
    jp->jsocks = NULL;
    jp->count = 0;
    return jp;
}

/*
 * Waits for events on JPoll instance. Up to maxevents
 * When successfully, j_poll_wait() returns a number of ready JSocket.
 * or zero if no JSocket became ready during the request timeout milliseconds
 * When an error occurs, returns -1
 */
gint j_poll_wait(JPoll * jp, JPollEvent * jevents, guint maxevents,
                 gint timeout)
{
    struct epoll_event events[128];
    if (maxevents > 128) {
        maxevents = 128;
    } else if (maxevents == 0) {
        return 0;
    }
    gint epollfd = j_poll_fd(jp);
    gint n;
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

    gint i;
    for (i = 0; i < n; i++) {
        jevents[i].type = events[i].events;
        jevents[i].jsock = (JSocket *) events[i].data.ptr;
    }
    return n;
}


/* Registers a JSocket */
gint j_poll_register(JPoll * jp, JSocket * jsock, guint32 events)
{
    gint epollfd = j_poll_fd(jp);
    gint sockfd = j_socket_fd(jsock);

    struct epoll_event event;
    event.events = events;      /* epoll_wait will always wait for EPOLLERR and EPOLLHUP, not necessary to set here */
    event.data.ptr = (void *) jsock;

    j_poll_add_jsocket(jp, jsock);

    return !epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
}

/*
 * Modify the event associated to the JSocket
 * Returns 1 on success, otherwise 0
 */
gint j_poll_modify(JPoll * jp, JSocket * jsock, guint32 events)
{
    gint epollfd = j_poll_fd(jp);
    gint sockfd = j_socket_fd(jsock);

    struct epoll_event event;
    event.events = events;
    event.data.ptr = (void *) jsock;

    return !epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &event);
}

/*
 * Unregisters the JSocket
 * Returns 1 on success, otherwise 0
 */
gint j_poll_delete(JPoll * jp, JSocket * jsock)
{
    gint epollfd = j_poll_fd(jp);
    gint sockfd = j_socket_fd(jsock);

    j_poll_remove_jsocket(jp, jsock);

    struct epoll_event event;   /* linux 2.6.9 required a non-null pointer in event */
    return !epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, &event);
}

/*
 * Unregisters the Jsocket and close it
 */
gint j_poll_delete_close(JPoll * jp, JSocket * jsock)
{
    gint ret = j_poll_delete(jp, jsock);
    j_socket_close(jsock);
    return ret;
}

/*
 * Closes JPoll
 * This function will close JPoll and free all the memory used by JPoll
 * But not free JSockets registered.
 * So get all JSockets registered before close the JPoll
 */
gint j_poll_close(JPoll * jp)
{
    gint epollfd = j_poll_fd(jp);

    gint ret = close(epollfd);
    g_slice_free1(sizeof(JPoll), jp);

    return ret;
}

/*
 * Closes JPoll and all JSocket registered
 */
gint j_poll_close_all(JPoll * jp)
{
    GList *jsocks = j_poll_all(jp);
    g_list_free_full(jsocks, (GDestroyNotify) j_socket_close);
    return j_poll_close(jp);
}

static inline JPollEvent *j_poll_event_new(struct epoll_event *event)
{
    JPollEvent *jpe = (JPollEvent *) g_slice_alloc(sizeof(JPollEvent));
    jpe->type = event->events;
    jpe->jsock = (JSocket *) event->data.ptr;
    return jpe;
}

static inline void j_poll_event_free(JPollEvent * jpe)
{
    g_slice_free1(sizeof(JPollEvent), jpe);
}

/*
 * Removes all JSockets that are not active during last timeout seconds
 */
void j_poll_remove_timeout(JPoll * jp, guint64 timeout)
{
    guint64 now = (guint64) time(NULL);
    GList *ptr = jp->jsocks;
    while (ptr) {
        JSocket *jsock = (JSocket *) ptr->data;
        GList *next = g_list_next(ptr);
        if (j_socket_active_time(jsock) + timeout < now) {
            GList *prev = ptr->prev;
            if (prev) {
                prev->next = next;
            } else {
                jp->jsocks = next;
            }
            if (next) {
                next->prev = prev;
            }
            g_list_free1(ptr);
            jp->count--;
        }
        ptr = next;
    }
}
