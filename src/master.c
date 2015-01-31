/*
 * master.h
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
#include "master.h"
#include "server.h"
#include "config.h"
#include "utils.h"
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

static JaMaster *gMaster = NULL;

static void signal_handler(int signum);
static void sigint_handler(void);

/*
 * initialize the signal handlers
 */
static void signal_initialize(void);

/*
 * Starts the master process of jacques
 * If fail, will call g_error() to terminate the process
 */
JaMaster *ja_master_create()
{
    JConfig *cfg = ja_config_load();
    ja_config_load_modules(cfg);

    GList *children = ja_server_load(cfg);
    gMaster = (JaMaster *) g_slice_alloc(sizeof(JaMaster));
    gMaster->cfg = cfg;
    gMaster->children = children;
    return gMaster;
}

void ja_master_wait(JaMaster * master)
{
    signal_initialize();
    GList *children = master->children;
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
    master->children = NULL;
}

void ja_master_quit(JaMaster * master)
{
    j_config_free(master->cfg);
    g_slice_free1(sizeof(JaMaster), master);
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
    GList *ptr = gMaster->children;
    while (ptr) {
        pid_t pid = (pid_t) (gulong) ptr->data;
        kill(pid, SIGINT);
        ptr = g_list_next(ptr);
    }

    while (wait(NULL) > 0) {
    }
    g_list_free(gMaster->children);
    gMaster->children = NULL;
    g_message("CORE QUIT");
}
