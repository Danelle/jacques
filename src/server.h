/*
 * server.h
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
#ifndef __JA_SERVER_H__
#define __JA_SERVER_H__

#include <glib.h>
#include "config.h"


typedef struct {
    gchar *name;                /* server name */
    gushort listen_port;
    guint32 max_pending;        /*  maximum length to which the  queue  of pending  connections */
    guint32 max_thread_count;   /* maximum count of threads */
    guint32 max_conn_per_thread;    /* maximum count of connection that one thread handles */
} JaServerConfig;

#define DIRECTIVE_LISTEN_PORT "ListenPort"
#define DIRECTIVE_MAX_PENDING "MaxPending"
#define DIRECTIVE_MAX_THREAD_COUNT "MaxThreadCount"
#define DIRECTIVE_MAX_CONN_PER_THREAD "MaxConnPerThread"

#define DEFAULT_MAX_PENDING 256
#define DEFAULT_MAX_THREAD_COUNT  0
#define DEFAULT_MAX_CONN_PER_THREAD 60


/*
 * Parses every file in CONFIG_APP_LOCATION
 * Creates a list of JaServerConfig
 */
GList *ja_server_config_load();





#endif                          /* __JA_SERVER_H__ */
