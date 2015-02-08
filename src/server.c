/*
 * server.c
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
#include "server.h"
#include "worker.h"
#include "config.h"
#include "utils.h"
#include "log.h"
#include <signal.h>
#include <unistd.h>
#include <glib/gi18n-lib.h>


static JaServer *gServer = NULL;

/*
 * Allocates memory for JaServer
 */
static inline JaServer *ja_server_alloc(const gchar * name,
                                        gint listen_port,
                                        gint max_pending,
                                        gint thread_count,
                                        JConfParser * cfg);

/*
 * The main loop of server process
 * This function will never return, if error occurs or signal catched, it will call _exit() but not return
 */
static inline void ja_server_main(JaServer * server);

/*
 * Invoke all hooks register as JA_HOOK_TYPE_SERVER_QUIT before exiting
 */
static inline void ja_server_quit(JaServer * server);


static inline void ja_server_create_from_file(const gchar * name,
                                              JConfParser * cfg);



/* Initialize signal handlers */
static void inline signal_initialize(void);
/* handle signals */
static void signal_handler(gint signum);
static void sigint_handler(void);

/*
 * Loads configuration of every server, and create server process
 * Return the list of server process id
 */
GList *ja_server_load(JConfParser * cfg)
{
    GError *err = NULL;
    GDir *dir = g_dir_open(CONFIG_APP_LOCATION, 0, &err);
    if (dir == NULL) {
        g_warning("fail to open %s: %s", CONFIG_APP_LOCATION,
                  err->message);
        g_error_free(err);
        return NULL;
    }
    GList *servers = NULL;
    const gchar *name = NULL;
    while ((name = g_dir_read_name(dir))) {
        pid_t pid = ja_server_create(name, cfg);
        if (pid < 0) {
            g_warning("fail to create server %s", name);
        } else {                /* parent */
            JaRunningServer *server = ja_running_server_new(pid, name);
            servers = g_list_append(servers, (void *) server);
        }
    }
    g_dir_close(dir);
    return servers;
}

pid_t ja_server_create(const gchar * name, JConfParser * cfg)
{
    pid_t pid = fork();
    if (pid < 0) {
        return (pid_t) - 1;
    } else if (pid > 0) {
        return pid;
    }
    ja_server_create_from_file(name, cfg);
    _exit(-1);
}

static inline void ja_server_create_from_file(const gchar * name,
                                              JConfParser * cfg)
{
    gchar buf[4096];
    g_snprintf(buf, sizeof(buf), "%s/%s", CONFIG_APP_LOCATION, name);
    if (!j_parse_more(cfg, buf, NULL)) {
        return;
    }

    /* ListenPort must be set */
    gint listen_port = j_parser_get_directive_integer(cfg,
                                                      DIRECTIVE_LISTEN_PORT);
    if (listen_port <= 0 || listen_port > 65536) {
        g_error("%s: Invalid ListenPort", name);
    }

    /* if MaxPending and ThreadCount are not set or invalid value is set
     * use default value instead */
    gint max_pending = j_parser_get_directive_integer(cfg,
                                                      DIRECTIVE_MAX_PENDING);
    gint thread_count = j_parser_get_directive_integer(cfg,
                                                       DIRECTIVE_THREAD_COUNT);
    if (max_pending <= 0) {
        max_pending = DEFAULT_MAX_PENDING;
    }
    if (thread_count <= 0) {
        thread_count = DEFAULT_THREAD_COUNT;
    }

    /* more configurations */

    const gchar *log_message = j_parser_get_directive_text(cfg,
                                                           DIRECTIVE_LOG_MESSAGE);
    const gchar *log_error = j_parser_get_directive_text(cfg,
                                                         DIRECTIVE_LOG_ERROR);
    set_custom_log(log_message, log_error);

    /* */

    gServer =
        ja_server_alloc(name, listen_port, max_pending, thread_count, cfg);

    /* Loads modules */
    ja_config_load_modules(cfg);

    ja_server_main(gServer);
}



static inline JaServer *ja_server_alloc(const gchar * name,
                                        gint listen_port,
                                        gint max_pending,
                                        gint thread_count,
                                        JConfParser * cfg)
{
    JaServer *server = (JaServer *) g_slice_alloc(sizeof(JaServer));
    server->name = g_strdup(name);
    server->listen_port = listen_port;
    server->max_pending = max_pending;
    server->thread_count = thread_count;
    server->listen_sock = NULL;
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
            /* a worker quits unexpectedly. restart it */
            g_warning(_("Server %s:Restart worker %d"), server->name,
                      ja_worker_get_id(jw));
            ptr->data =
                ja_worker_create(server->cfg, ja_worker_get_id(jw));
            if (ptr->data) {
                /* successfully */
                worker = jw;
            } else {            /* fail saddly */
                g_warning(_("Server %s:Fail to restart worker %d"),
                          server->name, ja_worker_get_id(jw));
                GList *prev = g_list_previous(ptr);
                if (ptr) {
                    prev->next = next;
                } else {
                    server->workers = next;
                }
                if (next) {
                    next->prev = prev;
                }
                g_list_free1(ptr);
            }
            ja_worker_free(jw);
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
            g_warning(_("Server %s:Fail to create worker"), server->name);
        }
    }
}


static void inline ja_server_initialize(JaServer * server);

#include <errno.h>
#include <string.h>
static inline void ja_server_main(JaServer * server)
{
    g_message("server : %s", server->name);
    g_message("\tListenPort:%d", server->listen_port);
    g_message("\tMaxPending:%d", server->max_pending);
    g_message("\tThreadCount:%d", server->thread_count);

    ja_server_initialize(server);

    JSocket *conn = NULL;
    while ((conn = j_socket_accept(server->listen_sock))) {
        JaWorker *worker = ja_server_find_worker(server);
        if (!worker) {
            j_socket_close(conn);
            g_warning("no worker found");
            continue;
        }
        ja_worker_add(worker, conn);
    }
    g_warning(_("Server %s quits unexpectedly: %s"),
              server->name, strerror(errno));
    ja_server_quit(server);
}

static void inline ja_server_initialize(JaServer * server)
{
    if (!setuser(CONFIG_USER)) {
        g_error(_("Unable to set user as %s"), CONFIG_USER);
    }
    signal_initialize();
    ja_server_initialize_workers(server);
    JSocket *jsock =
        j_server_socket_new(server->listen_port, server->max_pending);
    if (jsock == NULL) {
        _exit(-1);
    }
    server->listen_sock = jsock;

    set_proctitle((gchar **) NULL, "jacques: server %s", server->name);
}

static void inline signal_initialize(void)
{
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        g_error("fail to set SIGINT handler");
    }
}

static void signal_handler(gint signum)
{
    switch (signum) {
    case SIGINT:
        sigint_handler();
        break;
    }
}

static void sigint_handler(void)
{
    g_message("server %s quits", gServer->name);
    ja_server_quit(gServer);
}


static inline void ja_server_quit(JaServer * server)
{
    GList *hooks = ja_get_server_quit_hooks();
    while (hooks) {
        JaServerQuitHandler handler = (JaServerQuitHandler) hooks->data;
        handler(server->name, server->listen_port);
        hooks = g_list_next(hooks);
    }
    _exit(0);
}




/********* JaRunningServer **************************************/
JaRunningServer *ja_running_server_new(pid_t pid, const gchar * name)
{
    JaRunningServer *server =
        (JaRunningServer *) g_slice_alloc(sizeof(JaRunningServer));
    server->pid = pid;
    server->name = g_strdup(name);
    server->running = TRUE;
    return server;
}

void ja_running_server_free(JaRunningServer * server)
{
    g_free(server->name);
    g_slice_free1(sizeof(JaRunningServer), server);
}
