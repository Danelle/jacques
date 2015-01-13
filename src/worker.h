/*
 * worker.h
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

#ifndef __JA_WORKER_H__
#define __JA_WORKER_H__


#include "server.h"

/*
 * JaWorker - the thread worker
 */

typedef struct _JaWorker JaWorker;


/*
 * Creates an JaWorker, and run it
 * JaWorker is thread safe
 */
JaWorker *ja_worker_create(JaServerConfig * cfg);


/*
 * Adds a client to the worker
 */
void ja_worker_add(JaWorker *jw, JSocket *jsock);


/*
 * Check if worker is running
 */
gboolean ja_worker_is_running(JaWorker * jw);

/*
 * Check if worker is full load
 */
gboolean ja_worker_is_full(JaWorker * jw);

#endif
