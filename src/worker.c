/*
 * worker.c
 *
 * Copyright (C) 2015 Wiky L <wiiiky@outlook.com>
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
#include "config.h"
#include <jio.h>
#include <pthread.h>


#define DIRECTIVE_KEEPALIVE "KeepAlive"
#define DEFAULT_KEEPALIVE   5


struct _JaWorker {
    gint id;
    GThread *thread;
    JPoll *poller;
    gboolean running;

    gint keepalive;             /* negative means keepalive as long as possible, zero means no keepalive */

    GMutex lock;
};

#define ja_worker_lock(jw)  g_mutex_lock (&(jw)->lock)
#define ja_worker_unlock(jw) g_mutex_unlock (&(jw)->lock)


static inline JaWorker *ja_worker_alloc(JConfig * cfg, gint id);

/*
 * pthread routine
 */
static void *ja_worker_main(void *arg);


/*
 * Creates an JaWorker
 * JaWorker is thread safe
 */
JaWorker *ja_worker_create(JConfig * cfg, gint id)
{
    JaWorker *jw = ja_worker_alloc(cfg, id);
    if (jw == NULL) {
        return NULL;
    }
    jw->thread =
        g_thread_try_new("worker", (GThreadFunc) ja_worker_main,
                         (void *) jw, NULL);
    if (jw->thread == NULL) {   /* error */
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
    g_message("worker %d: new socket", jw->id);
}

static inline void ja_worker_modify(JaWorker * jw, JSocket * jsock,
                                    guint32 events)
{
    ja_worker_lock(jw);
    j_poll_modify(jw->poller, jsock, events);
    ja_worker_unlock(jw);
}

/*
 * Deletes a JSocket
 * and closes it
 */
static inline void ja_worker_remove(JaWorker * jw, JSocket * jsock)
{
    ja_worker_lock(jw);
    j_poll_delete_close(jw->poller, jsock);
    ja_worker_unlock(jw);
    g_message("worker %d: close socket", jw->id);
}


static inline gint ja_worker_send(JaWorker * jw, JSocket * jsock,
                                  const void *data, guint32 count)
{
    gint n = j_socket_write(jsock, data, count);
    if (n < 0) {
        ja_worker_remove(jw, jsock);
    } else if (n == 1) {
        ja_worker_modify(jw, jsock, J_POLL_EVENT_IN);
    } else {
        JaAction act = j_socket_get_flag(jsock);
        if ((!(act & JA_ACTION_KEEP) && jw->keepalive == 0) || (act & JA_ACTION_DROP)) {    /* should not keep the connection */
            ja_worker_remove(jw, jsock);
        }
    }

    return n;
}

static inline void ja_worker_handle_request(JaWorker * jw, JSocket * jsock)
{
    const void *data = j_socket_data(jsock);
    guint length = j_socket_data_length(jsock);
    JaRequest *req = ja_request_new(data, length, NULL, 0);


    JaAction act = JA_ACTION_IGNORE;

    GList *hooks = ja_get_request_hooks();

    while (hooks) {
        JaRequestHandler func = (JaRequestHandler) hooks->data;
        act = func(req);
        if (act & JA_ACTION_DROP || act & JA_ACTION_IGNORE) {
            break;
        }
        hooks = g_list_next(hooks);
    }

    /* action */
    if (act & JA_ACTION_RESPONSE) {
        data = ja_response_data(req);
        length = ja_response_data_length(req);
        if (ja_worker_send(jw, jsock, data, length) == 0) { /* response */
            ja_worker_modify(jw, jsock, J_POLL_EVENT_OUT);
            j_socket_set_flag(jsock, act);  /* JaAction after writing completely */
            ja_request_free(req);
            return;
        }
    }

    if ((!(act & JA_ACTION_KEEP) && jw->keepalive == 0) || (act & JA_ACTION_DROP)) {    /* should not keep the connection */
        ja_worker_remove(jw, jsock);
    }

    ja_request_free(req);
}


/*
 * Removes all connections that is timeout
 */
static inline void ja_worker_timeout(JaWorker * jw)
{
    if (jw->keepalive < 0) {
        return;
    }
    guint64 timeout =
        jw->keepalive == 0 ? DEFAULT_KEEPALIVE : jw->keepalive;
    ja_worker_lock(jw);
    guint32 count = j_poll_remove_timeout(jw->poller, timeout);
    if (count > 0) {
        g_message("%d Jsocket(s) timeout and removed", count);
    }
    ja_worker_unlock(jw);
}


/*
 * thread routine!!!
 */
static void *ja_worker_main(void *arg)
{
    JaWorker *jw = (JaWorker *) arg;
    JPoll *poller = jw->poller;
    gint i, n;
    g_message("new worker:%d", jw->id);
    JPollEvent events[128];
    while ((n =
            j_poll_wait(poller, events,
                        sizeof(events) / sizeof(JPollEvent), 1000)) >= 0) {
        if (n > 0) {
            for (i = 0; i < n; i++) {
                JSocket *jsock = events[i].jsock;
                guint32 type = events[i].type;
                if (type & J_POLL_EVENT_IN) {   /* ready for reading */
                    gint n = j_socket_read(jsock);
                    if (n < 0) {    /* read error, EOF? whatever */
                        ja_worker_remove(jw, jsock);
                    } else if (n > 0) {
                        ja_worker_handle_request(jw, jsock);
                    }
                } else if (type & J_POLL_EVENT_OUT) {   /* ready for writing */
                    ja_worker_send(jw, jsock, NULL, 0);
                } else if (type & (J_POLL_EVENT_HUP | J_POLL_EVENT_ERR)) {
                    /* error */
                    ja_worker_remove(jw, jsock);
                }
            }
        }
        ja_worker_timeout(jw);
    }

    jw->running = FALSE;
    g_warning("worker quits");
    return (void *) 0;
}


/*
 * Check if thread is running
 */
gboolean ja_worker_is_running(JaWorker * jw)
{
    return jw->running;
}

guint32 ja_worker_payload(JaWorker * jw)
{
    return j_poll_count(jw->poller);
}

static inline JaWorker *ja_worker_alloc(JConfig * cfg, gint id)
{
    JPoll *poller = j_poll_new();
    if (poller == NULL) {
        return NULL;
    }
    JaWorker *jw = (JaWorker *) g_slice_alloc(sizeof(JaWorker));
    jw->id = id;
    jw->poller = poller;
    jw->running = TRUE;
    jw->keepalive = j_config_get_integer(cfg, NULL, DIRECTIVE_KEEPALIVE);
    g_mutex_init(&jw->lock);
    return jw;
}

void ja_worker_free(JaWorker * jw)
{
    j_poll_close_all(jw->poller);
    if (jw->thread) {
        g_thread_unref(jw->thread);
    }
    g_slice_free1(sizeof(JaWorker), jw);
}
