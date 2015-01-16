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
#include <unistd.h>


static inline JaServerConfig *ja_server_config_default(const gchar * name,
                                                       gushort port);
static inline void ja_server_config_free(JaServerConfig * cfg);


/*
 * Allocates memory for JaServer
 */
static inline JaServer *ja_server_alloc(JSocket * jsock,
                                        JaServerConfig * cfg);
/*
 * The main loop of server process
 * This function will never return, if error occurs or signal catched, it may _exit() but not return
 */
static inline void ja_server_main(JaServer * server);

/*
 * Constructs a JaServerConfig
 */
static inline JaServerConfig *ja_server_config_parse(JConfig * cfg,
                                                     JDirectiveGroup *
                                                     global);


/*
 * Parses every file in CONFIG_APP_LOCATION
 * Creates a list of JaServerConfig
 */
GList *ja_server_config_load(JDirectiveGroup * global)
{
    GError *err = NULL;
    GDir *dir = g_dir_open(CONFIG_APP_LOCATION, 0, &err);
    if (dir == NULL) {
        g_warning("fail to open %s: %s", CONFIG_APP_LOCATION,
                  err->message);
        g_error_free(err);
        return NULL;
    }
    GList *ret = NULL;
    const gchar *name = NULL;
    gchar pathbuf[4096];        /* 4096 must be enough */
    while ((name = g_dir_read_name(dir))) {
        g_snprintf(pathbuf, sizeof(pathbuf), "%s/%s", CONFIG_APP_LOCATION,
                   name);
        JConfig *cfg = j_conf_parse(pathbuf);
        JaServerConfig *scfg = ja_server_config_parse(cfg, global);
        if (scfg) {
            ret = g_list_append(ret, scfg);
        }
    }
    g_dir_close(dir);

    return ret;
}

/*
 * Free all the memory used by JaServerConfig GList 
 */
void ja_server_config_free_all(GList * scfgs)
{
    g_list_free_full(scfgs, (GDestroyNotify) ja_server_config_free);
}


static inline JaServerConfig *ja_server_config_parse(JConfig * cfg,
                                                     JDirectiveGroup *
                                                     global)
{
    gint port = j_config_get_integer(cfg, NULL, DIRECTIVE_LISTEN_PORT);
    if (port <= 0 || port > 65535) {
        g_warning("%s not found in %s", DIRECTIVE_LISTEN_PORT, cfg->name);
        return NULL;
    }
    JaServerConfig *scfg = ja_server_config_default(cfg->name, port);
    gint max_pending;
    if ((max_pending =
         j_config_get_integer(cfg, NULL, DIRECTIVE_MAX_PENDING)) > 0
        || (max_pending =
            j_directive_group_get_integer(global,
                                          DIRECTIVE_MAX_PENDING)) > 0) {
        scfg->max_pending = max_pending;
    }

    gint thread_count;
    if ((thread_count =
         j_config_get_integer(cfg, NULL, DIRECTIVE_THREAD_COUNT)) > 0
        || (thread_count =
            j_directive_group_get_integer(global,
                                          DIRECTIVE_THREAD_COUNT)) >= 0) {
        scfg->thread_count = thread_count;
    }

    scfg->cfg = cfg;
    return scfg;
}


static inline JaServerConfig *ja_server_config_default(const gchar * name,
                                                       gushort port)
{
    JaServerConfig *cfg = g_slice_alloc(sizeof(JaServerConfig));
    cfg->name = g_strdup(name);
    cfg->listen_port = port;
    cfg->max_pending = DEFAULT_MAX_PENDING;
    cfg->thread_count = DEFAULT_THREAD_COUNT;
    cfg->cfg = NULL;
    return cfg;
}

static inline void ja_server_config_free(JaServerConfig * cfg)
{
    g_free(cfg->name);
    j_config_free(cfg->cfg);
    g_slice_free1(sizeof(JaServerConfig), cfg);
}



static inline JaServer *ja_server_alloc(JSocket * jsock,
                                        JaServerConfig * cfg)
{
    JaServer *server = (JaServer *) g_slice_alloc(sizeof(JaServer));
    server->listen_sock = jsock;
    server->cfg = cfg;
    server->workers = NULL;
    return server;
}


/*
 * Creates a JaServer, (fork a new process)
 * Returns fork()
 */
gint ja_server_create(JaServerConfig * cfg)
{
    pid_t pid = fork();
    if (pid < 0) {
        return (gint) pid;
    } else if (pid > 0) {       /* parent */
        return (gint) pid;
    }

    JSocket *jsock =
        j_server_socket_new(cfg->listen_port, cfg->max_pending);
    if (jsock == NULL) {
        _exit(-1);
    }

    JaServer *server = ja_server_alloc(jsock, cfg);
    ja_server_main(server);

    /* never come here */
    return -1;
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
    gint count = server->cfg->thread_count;
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
    g_message
        ("server %s starts! \n\t\tmax_pending=%d\n\t\tthread_count=%d",
         server->cfg->name, server->cfg->max_pending,
         server->cfg->thread_count);
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
    /* error */
    _exit(0);
}
