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
#include "config.h"
#include "utils.h"
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

static JaCore *gCore = NULL;

static void signal_handler(int signum);
static void sigint_handler(void);

/*
 * initialize the signal handlers
 */
static void signal_initialize(void);

/*
 * Starts the core process of jacques
 * If fail, will call g_error() to terminate the process
 */
JaCore *ja_core_create()
{
    JConfig *cfg = ja_config_load();
    ja_config_load_modules(cfg);

    GList *children = ja_server_load(cfg);
    gCore = (JaCore *) g_slice_alloc(sizeof(JaCore));
    gCore->cfg = cfg;
    gCore->children = children;
    return gCore;
}

void ja_core_wait(JaCore * core)
{
    signal_initialize();
    GList *children = core->children;
    int status;
    while (1) {
        pid_t pid = wait(&status);
        if (pid > 0) {
            children = g_list_remove(children, (void *) (gulong) pid);
            g_warning("server %d: status %d", pid, status);
        } else if (pid < 0 && errno == ECHILD) {
            break;
        }
    }
    core->children = NULL;
}

void ja_core_quit(JaCore * core)
{
    j_config_free(core->cfg);
    g_slice_free1(sizeof(JaCore), core);
}


static void signal_initialize(void)
{
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        g_warning("fail to set SIGINT handler!");
    }
}

static void signal_handler(int signum)
{
    switch (signum) {
    case SIGINT:
        sigint_handler();
        break;
    }
}

static void sigint_handler(void)
{
    if (gCore == NULL) {
        return;
    }
    GList *ptr = gCore->children;
    while (ptr) {
        pid_t pid = (pid_t) (gulong) ptr->data;
        kill(pid, SIGINT);
        ptr = g_list_next(ptr);
    }

    while (wait(NULL) > 0) {
    }
    g_list_free(gCore->children);
    gCore->children = NULL;
    g_message("CORE QUIT");
}
