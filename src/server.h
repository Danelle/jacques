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
#include "io/jio.h"
#include "config.h"


/*
 * A server is a process that listen for connection and handle
 */


typedef struct {
    gchar *name;                /* server name */
    gushort listen_port;
    guint32 max_pending;        /*  maximum length to which the  queue  of pending  connections */
    guint32 thread_count;       /* count of workers */

    JConfig *cfg;
} JaServerConfig;

#define DIRECTIVE_LISTEN_PORT "ListenPort"
#define DIRECTIVE_MAX_PENDING "MaxPending"
#define DIRECTIVE_THREAD_COUNT "ThreadCount"

#define DEFAULT_MAX_PENDING 256
#define DEFAULT_THREAD_COUNT  6


/*
 * Parses every file in CONFIG_APP_LOCATION
 * Creates a list of JaServerConfig
 */
GList *ja_server_config_load(JDirectiveGroup * global);


/*
 * Free all the memory used by JaServerConfig GList 
 */
void ja_server_config_free_all(GList * scfgs);



typedef struct {
    JSocket *listen_sock;
    GList *workers;             /* the list of worker thread */
    JaServerConfig *cfg;
} JaServer;

/*
 * Creates a JaServer, (fork a new process)
 * Returns fork()
 * Child process will not return
 */
gint ja_server_create(JaServerConfig * cfg);




#endif                          /* __JA_SERVER_H__ */
