/*
 * server.c
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
#include "server.h"
#include "worker.h"
#include "mod.h"
#include "config.h"
#include <unistd.h>


/*
 * Allocates memory for JaServer
 */
static inline JaServer *ja_server_alloc(const gchar * name,
                                        gint port,
                                        gint max_pending,
                                        gint thread_count,
                                        JSocket * jsock, JConfig * cfg);

/*
 * The main loop of server process
 * This function will never return, if error occurs or signal catched, it may _exit() but not return
 */
static inline void ja_server_main(JaServer * server);


static inline void ja_server_create_from_file(const gchar * name,
                                              JConfig * cfg);


/*
 * Loads configuration of every server, and create server process
 * Return the list of server process id
 */
GList *ja_server_load(JConfig * cfg)
{
    GError *err = NULL;
    GDir *dir = g_dir_open(CONFIG_APP_LOCATION, 0, &err);
    if (dir == NULL) {
        g_warning("fail to open %s: %s", CONFIG_APP_LOCATION,
                  err->message);
        g_error_free(err);
        return NULL;
    }
    GList *pids = NULL;
    const gchar *name = NULL;
    while ((name = g_dir_read_name(dir))) {
        pid_t pid = fork();
        if (pid < 0) {
            g_warning("fail to create server %s", name);
        } else if (pid == 0) {  /* server */
            ja_server_create_from_file(name, cfg);
        } else {                /* parent */
            pids = g_list_append(pids, (void *) (gulong) pid);
        }
    }
    g_dir_close(dir);
    return pids;
}

static inline void ja_server_create_from_file(const gchar * name,
                                              JConfig * cfg)
{
    gchar buf[4096];
    g_snprintf(buf, sizeof(buf), "%s/%s", CONFIG_APP_LOCATION, name);
    j_conf_parse_internal(buf, cfg);
    gint listen_port =
        j_config_get_integer(cfg, NULL, DIRECTIVE_LISTEN_PORT);
    if (listen_port <= 0 || listen_port > 65536) {
        g_error("%s: Invalid ListenPort", name);
    }
    gint max_pending =
        j_config_get_integer(cfg, NULL, DIRECTIVE_MAX_PENDING);
    gint thread_count =
        j_config_get_integer(cfg, NULL, DIRECTIVE_THREAD_COUNT);
    if (max_pending <= 0) {
        max_pending = DEFAULT_MAX_PENDING;
    }
    if (thread_count <= 0) {
        thread_count = DEFAULT_THREAD_COUNT;
    }

    JSocket *jsock = j_server_socket_new(listen_port, max_pending);
    if (jsock == NULL) {
        g_error("%s: fail to create socket!", name);
    }

    JaServer *server =
        ja_server_alloc(name, listen_port, max_pending, thread_count,
                        jsock, cfg);

    ja_server_main(server);

    _exit(-1);
    /* no */
}



static inline JaServer *ja_server_alloc(const gchar * name,
                                        gint port,
                                        gint max_pending,
                                        gint thread_count,
                                        JSocket * jsock, JConfig * cfg)
{
    JaServer *server = (JaServer *) g_slice_alloc(sizeof(JaServer));
    server->name = g_strdup(name);
    server->listen_port = port;
    server->max_pending = max_pending;
    server->thread_count = thread_count;
    server->listen_sock = jsock;
    server->cfg = cfg;
    server->workers = NULL;
    return server;
}

/*
 * find a worker that can handle more connection
 * if no existing one found, and more worker is allowed to create, then create a new one
 * otherwise return NULL
 */
static inline JaWorker *ja_server_find_worker(JaServer * server)
{
    GList *ptr = server->workers;
    JaWorker *worker = NULL;
    while (ptr) {
        GList *next = g_list_next(ptr);
        JaWorker *jw = (JaWorker *) ptr->data;
        if (!ja_worker_is_running(jw)) {
            if (next) {
                next->prev = ptr->prev;
            }

            if (ptr->prev == NULL) {
                server->workers = next;
            } else {
                ptr->prev->next = next;
            }
            g_list_free1(ptr);
            ja_worker_free(jw);
            g_warning("remove worker");
        } else if (worker == NULL
                   || ja_worker_payload(worker) > ja_worker_payload(jw)) {
            worker = jw;
        }
        ptr = next;
    }
    return worker;
}

static inline void ja_server_initialize_workers(JaServer * server)
{
    gint count = server->thread_count;
    gint i = 0;
    for (i = 0; i < count; i++) {
        JaWorker *worker = ja_worker_create(server->cfg, i);
        if (worker) {
            server->workers = g_list_prepend(server->workers, worker);
        } else {
            g_warning("fail to create worker");
        }
    }
}


#include <errno.h>
#include <string.h>
static inline void ja_server_main(JaServer * server)
{
    g_message("server : %s", server->name);
    g_message("\tListenPort:%d", server->listen_port);
    g_message("\tMaxPending:%d", server->max_pending);
    g_message("\tThreadCount:%d", server->thread_count);
    ja_server_initialize_workers(server);
    JSocket *conn = NULL;
    while ((conn = j_socket_accept(server->listen_sock))) {
        JaWorker *worker = ja_server_find_worker(server);
        if (!worker) {
            j_socket_close(conn);
            g_warning("~~~~~");
            continue;
        }
        ja_worker_add(worker, conn);
    }
    g_warning("server quits: %s", strerror(errno));
}
