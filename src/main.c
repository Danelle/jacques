/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
 * Copyright (C) 2015 Wiky L <wiiiky@outlook.com>
 *
 * Jacques is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Jacques is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "master.h"
#include "utils.h"
#include "log.h"
#include "err.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <glib/gprintf.h>
#include <glib/gi18n-lib.h>


static inline void show_version(void);
static inline void show_help(void);


static inline JaConfig *read_config(void);

/* Starts/Stops/Restarts jacques daemon */
static inline void start_jacques(void);
static inline void stop_jacques(void);
static inline void restart_jacques(void);


static void initialize(void)
{
    setlocale(LC_ALL, "");
}

int main(gint argc, const gchar * argv[])
{
    initialize();
    static struct option long_options[] = {
        {"version", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'},
        {"signal", 1, NULL, 's'},
        {NULL, 0, NULL, 0}
    };

    gint ch;
    while ((ch =
            getopt_long(argc, (char *const *) argv, "s:vh", long_options,
                        NULL)) != -1) {
        switch (ch) {
        case 'v':
            show_version();
            break;
        case 's':
            if (g_strcmp0(optarg, "stop") == 0) {
                stop_jacques();
            } else if (g_strcmp0(optarg, "restart") == 0) {
                restart_jacques();
            }
            break;
        case 'h':
        default:
            show_help();
            break;
        }
    }

    if (optind != argc) {
        show_help();
    }

    set_proctitle((gchar **) argv, "jacques: master");
    start_jacques();

    return EXIT_MASTER_OK;
}

static void inline show_version(void)
{
    g_printf(_("jacques version: %u.%u\n\n"), JACQUES_VERSION_MAJOR,
             JACQUES_VERSION_MINOR);
    g_printf(_("Build Infomation:\n"));
    g_printf(_("\tConfiguration Location: %s\n"), CONFIG_LOCATION);
    g_printf(_("\tLog Location: %s\n"), CONFIG_LOG_LOCATION);
    g_printf(_("\tRuntime Location: %s\n"), CONFIG_RUNTIME_LOCATION);
    exit(EXIT_MASTER_OK);
}

static void inline show_help(void)
{
    g_printf(_("Usage: jacques [option]\n\n"));
    g_printf(_("Options:\n"));
    g_printf(_("\t--help\t\t-h\tshow this help info.\n"));
    g_printf(_("\t--version\t-v\tshow the version of jacques.\n"));
    g_printf(_("\t--signal\t-s\tsend signal to the "
               "master process: stop, retart.\n"));
    exit(EXIT_MASTER_OK);
}


static inline JaConfig *initialize_jacques(void)
{
    JaConfig *cfg = read_config();
    g_mkdir_with_parents(CONFIG_RUNTIME_LOCATION, 0755);
    g_mkdir_with_parents(CONFIG_LOG_LOCATION, 0755);
    g_mkdir_with_parents(CONFIG_LOG_LOCATION, 0755);
    gint running = already_running();
    if (running > 0) {
        g_printf(_("jacqueas is already running!!!\n"));
        exit(EXIT_MASTER_ALREADY_RUNNING);
    } else if (running < 0) {
        g_printf(_("Unable to create pid file:%s\n"), strerror(errno));
        exit(EXIT_MASTER_INITIALIZE);
    } else if (!daemonize() || !initialize_default_log()
               || !lock_pidfile()) {
        g_printf(_("Unable to initialize jacques:%s\n"), strerror(errno));
        exit(EXIT_MASTER_INITIALIZE);
    }
    return cfg;
}

static void inline start_jacques(void)
{
    JaConfig *cfg = initialize_jacques();

    JaMaster *master = ja_master_create(cfg);
    ja_master_wait(master);
    ja_master_quit(master);
}

static void inline stop_jacques(void)
{
    gint running = already_running();
    if (running == 0) {
        g_printf(_("jacques is not running!!!\n"));
        exit(0);
    } else if (running < 0) {
        g_printf(_("Unable to get jacques process id:%s\n"),
                 strerror(errno));
        exit(0);
    }
    if (kill(running, SIGINT)) {
        g_printf(_("Fail to send signal SIGINT to process %d:%s\n"),
                 running, strerror(errno));
    } else {
        g_printf(("Send signal SIGINT to jacques: %d\n"), running);
    }
    exit(EXIT_MASTER_OK);
}

static void inline restart_jacques(void)
{
}


static inline JaConfig *read_config(void)
{
    GError *error = NULL;
    JaConfig *cfg = ja_config_load(&error);

    if (cfg == NULL) {
        g_printf("fail to start jacques:%s\n", error->message);
        g_error_free(error);
        exit(EXIT_INVALID_CONFIG);
    }
    return cfg;
}
