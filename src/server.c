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
static inline JaServerConfig *ja_server_config_parse(GKeyFile * kf,
                                                     const gchar * group);


/*
 * Parses every file in CONFIG_APP_LOCATION
 * Creates a list of JaServerConfig
 */
GList *ja_server_config_load()
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
        GKeyFile *kf = g_key_file_new();
        if (!g_key_file_load_from_file(kf, pathbuf, 0, NULL)) {
            g_key_file_free(kf);
            g_warning("fail to parse %s", pathbuf);
            continue;
        }
        gchar **groups = g_key_file_get_groups(kf, NULL);
        guint i = 0;
        while (groups[i]) {
            JaServerConfig *scfg = ja_server_config_parse(kf, groups[i]);
            if (scfg) {
                ret = g_list_append(ret, scfg);
            }
            i++;
        }
        g_strfreev(groups);
        g_key_file_unref(kf);
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

static inline JaServerConfig *ja_server_config_parse(GKeyFile * kf,
                                                     const gchar * group)
{
    const gchar *name = group;
    gint port =
        g_key_file_get_integer(kf, group, DIRECTIVE_LISTEN_PORT, NULL);
    if (port <= 0 || port > 65535) {
        g_warning("server %s has an invalid port number", group);
        return NULL;
    }
    JaServerConfig *cfg = ja_server_config_default(name, port);

    gint max_pending =
        g_key_file_get_integer(kf, group, DIRECTIVE_MAX_PENDING, NULL);
    if (max_pending > 0) {
        cfg->max_pending = max_pending;
    }
    gint max_thread_count =
        g_key_file_get_integer(kf, group, DIRECTIVE_MAX_THREAD_COUNT,
                               NULL);
    if (max_thread_count > 0) {
        cfg->max_thread_count = max_thread_count;
    }
    gint max_conn_per_thread =
        g_key_file_get_integer(kf, group, DIRECTIVE_MAX_CONN_PER_THREAD,
                               NULL);
    if (max_conn_per_thread > 0) {
        cfg->max_thread_count = max_conn_per_thread;
    }
    return cfg;
}



static inline JaServerConfig *ja_server_config_default(const gchar * name,
                                                       gushort port)
{
    JaServerConfig *cfg = g_slice_alloc(sizeof(JaServerConfig));
    cfg->name = g_strdup(name);
    cfg->listen_port = port;
    cfg->max_pending = DEFAULT_MAX_PENDING;
    cfg->max_thread_count = DEFAULT_MAX_THREAD_COUNT;
    cfg->max_conn_per_thread = DEFAULT_MAX_CONN_PER_THREAD;
    return cfg;
}

static inline void ja_server_config_free(JaServerConfig * cfg)
{
    g_free(cfg->name);
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
        JaWorker *jw = (JaWorker *) ptr->data;
        if (!ja_worker_is_full(jw)) {
            worker = jw;
            break;
        }
        ptr = g_list_next(ptr);
    }
    if (worker) {
        return worker;
    }
    worker = ja_worker_create(server->cfg);
    if (worker) {
        server->workers = g_list_append(server->workers, worker);
    }
    return worker;
}


static inline void ja_server_main(JaServer * server)
{
    JSocket *conn = NULL;
    while ((conn = j_socket_accept(server->listen_sock))) {
        JaWorker *worker = ja_server_find_worker(server);
        if (!worker) {
            j_socket_close(conn);
            g_warning("~~~~~");
            continue;
        }
        ja_worker_add(worker, conn);
        g_message("new socket");
    }
    /* error */
    _exit(0);
}
