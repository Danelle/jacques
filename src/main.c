/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
 * Copyright (C) 2015 Wiky L <wiiiky@yeah.net>
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

#include "core.h"
#include "utils.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <glib/gprintf.h>
#include <getopt.h>

static void inline show_version(void);
static void inline show_help(void);

static void inline start_jacques(void);
static void inline stop_jacques(void);
static void inline restart_jacques(void);

int main(int argc, const char *argv[])
{
    static struct option long_options[] = {
        {"version", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'},
        {"signal", 1, NULL, 's'},
        {NULL, 0, NULL, 0}
    };

    int ch;
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

    start_jacques();

    return (0);
}

static void inline show_version(void)
{
    g_printf("jacques version: %u.%u\n\n", JACQUES_VERSION_MAJOR,
             JACQUES_VERSION_MINOR);
    g_printf("Build Infomation:\n");
    g_printf("\tConfiguration Location: %s\n", CONFIG_LOCATION);
    g_printf("\tLog Location: %s\n", DEFAULT_LOG_LOCATION);
    g_printf("\tRuntime Location: %s\n", RUNTIME_LOCATION);
    exit(0);
}

static void inline show_help(void)
{
    g_printf("Usage: jacques [option]\n\n");
    g_printf("Options:\n");
    g_printf("\t--help\t\t-h\tshow this help info.\n");
    g_printf("\t--version\t-v\tshow the version of jacques.\n");
    g_printf
        ("\t--signal\t-s\tsend signal to the master process: stop, retart.\n");
    exit(0);
}

static void inline start_jacques(void)
{
    daemonize();

    int running = already_running();
    if (running > 0) {
        g_error("jacqueas is already running!!!");
        exit(0);
    } else if (running < 0) {
        g_error("fail to open pid file!!!");
        exit(0);
    }

    JaCore *core = ja_core_create();
    ja_core_wait(core);
    ja_core_quit(core);
}

static void inline stop_jacques(void)
{
    int running = already_running();
    if (running == 0) {
        g_printf("jacques is not running!!!");
        exit(0);
    } else if (running < 0) {
        g_printf("fail to open pid file!!!");
        exit(0);
    }
    if (kill(running, SIGINT)) {
        g_printf("fail to send signal SIGINT to process %d:%s", running,
                 strerror(errno));
    } else {
        g_printf("send signal SIGINT to process %d", running);
    }
    exit(0);
}

static void inline restart_jacques(void)
{
}
