/*
 * worker.c
 *
 * Copyright (C) 2015 Wiky L <wiiiky@yeah.net>
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


#include "worker.h"
#include "jio/jio.h"
#include <pthread.h>


struct _JaWorker {
    pthread_t tid;              /* thread id */
    JPoll *poller;
    guint max_client;
    gboolean running;

    pthread_mutex_t lock;
};

#define ja_worker_lock(jw)  pthread_mutex_lock(&(jw)->lock)
#define ja_worker_unlock(jw) pthread_mutex_unlock(&(jw)->lock)


static inline JaWorker *ja_worker_alloc(JaServerConfig * cfg);
static inline void ja_worker_free(JaWorker * jw);

/*
 * pthread routine
 */
static void *ja_worker_main(void *arg);


/*
 * Creates an JaWorker
 * JaWorker is thread safe
 */
JaWorker *ja_worker_create(JaServerConfig * cfg)
{
    JaWorker *jw = ja_worker_alloc(cfg);
    if (jw == NULL) {
        return NULL;
    }
    int ret = pthread_create(&jw->tid, NULL, ja_worker_main, (void *) jw);
    if (ret != 0) {             /* error */
        ja_worker_free(jw);
        return NULL;
    }
    return jw;
}

/*
 * Adds a client to the worker
 */
void ja_worker_add(JaWorker * jw, JSocket * jsock)
{
    ja_worker_lock(jw);
    JPoll *poller = jw->poller;
    j_poll_register(poller, jsock, J_POLL_EVENT_IN);
    ja_worker_unlock(jw);
}

/*
 * Deletes a JSocket
 * and closes it
 */
static inline void ja_worker_remove(JaWorker * jw, JSocket * jsock)
{
    ja_worker_lock(jw);
    JPoll *poller = jw->poller;
    j_poll_delete(poller, jsock);
    ja_worker_unlock(jw);
}


/*
 * thread routine!!!
 */
static void *ja_worker_main(void *arg)
{
    JaWorker *self = (JaWorker *) arg;
    JPoll *poller = self->poller;
    gint n;
    while ((n = j_poll_wait(poller, 128, 0)) >= 0) {
        if (n == 0) {
            continue;
        }
        GList *ready = j_poll_ready(poller);
        while (ready) {         /* new request */
            JPollEvent *event = (JPollEvent *) ready->data;
            if (event->type & J_POLL_EVENT_IN) {    /* ready for reading */
            } else if (event->type & J_POLL_EVENT_OUT) {    /* ready for writing */
            } else if (event->type & (J_POLL_EVENT_HUP | J_POLL_EVENT_ERR)) {
                /* error */
                ja_worker_remove(self, event->jsock);
            }
            ready = g_list_next(ready);
        }
    }

    self->running = FALSE;
    return (void *) 0;
}


/*
 * Check if thread is running
 */
gboolean ja_worker_is_running(JaWorker * jw)
{
    return jw->running;
}


/*
 * Check if worker is full load
 */
gboolean ja_worker_is_full(JaWorker * jw)
{
    if (jw->max_client <= 0) {
        return FALSE;
    }
    return j_poll_count(jw->poller) > jw->max_client;
}


static inline JaWorker *ja_worker_alloc(JaServerConfig * cfg)
{
    JPoll *poller = j_poll_new();
    if (poller == NULL) {
        return NULL;
    }
    JaWorker *jw = (JaWorker *) g_slice_alloc(sizeof(JaWorker));
    jw->poller = poller;
    jw->running = TRUE;
    jw->max_client = cfg->max_conn_per_thread;
    pthread_mutex_init(&(jw->lock), NULL);
    return jw;
}

static inline void ja_worker_free(JaWorker * jw)
{
    j_poll_close_all(jw->poller);
    g_slice_free1(sizeof(JaWorker), jw);
}
