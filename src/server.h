/*
 * server.h
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
#ifndef __JA_SERVER_H__
#define __JA_SERVER_H__

#include <glib.h>
#include <jio.h>
#include <pthread.h>
#include "config.h"


/*
 * A server is a process that listen for connection and handle
 */


#define DIRECTIVE_LISTEN_PORT "ListenPort"
#define DIRECTIVE_MAX_PENDING "MaxPending"
#define DIRECTIVE_THREAD_COUNT "ThreadCount"
#define DIRECTIVE_LOG_MESSAGE "LogMessage"
#define DIRECTIVE_LOG_ERROR "LogError"

#define DEFAULT_MAX_PENDING 256
#define DEFAULT_THREAD_COUNT  1


/*
 * Loads configuration of every server, and create server process
 * Return the list of server process id
 */
GList *ja_server_load(JConfig * cfg);



typedef struct {
    gchar *name;
    gint listen_port;
    gint max_pending;
    gint thread_count;

    JSocket *listen_sock;
    GList *workers;             /* the list of worker thread */
    JConfig *cfg;
} JaServer;

/* Forks a server, Returns the server pid on success, otherwise returns -1 */
pid_t ja_server_create(const gchar * name, JConfig * cfg);


#endif                          /* __JA_SERVER_H__ */
