/*
 * core.c
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
#include "core.h"
#include "server.h"
#include <sys/wait.h>



/*
 * Starts the core process of jacques
 * If fail, will call g_error() to terminate the process
 */
JaCore *ja_core_create()
{
    JConfig *cfg = ja_config_load();

    GList *children = ja_server_load(cfg);
    JaCore *core = (JaCore *) g_slice_alloc(sizeof(JaCore));
    core->cfg = cfg;
    core->children = children;
    return core;
}

void ja_core_wait(JaCore * core)
{
    GList *children = core->children;
    int status;
    while (g_list_length(children) > 0) {
        gint pid = wait(&status);
        if (pid > 0) {
            children = g_list_remove(children, (void *) (glong) pid);
            g_warning("server %d: status %d", pid, status);
        }
    }
    core->children = NULL;
}

void ja_core_quit(JaCore * core)
{
    j_config_free(core->cfg);
    g_slice_free1(sizeof(JaCore), core);
}
