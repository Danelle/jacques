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

#include "config.h"
#include "server.h"
#include "jio/jio.h"

int main(int argc, const char *argv[])
{
    JaConfig *jcfg = ja_config_load();
    if (jcfg == NULL) {
        g_printf("fail to parse configuration file\n");
        return -1;
    }

    JaDirectiveGroup *jdg_core = ja_config_lookup(jcfg, "core");
    if (jdg_core == NULL) {
        g_printf("you must set [core] in %s\n", CONFIG_FILEPATH);
        ja_config_free(jcfg);
        return -1;
    }

    GList *servers = ja_server_config_load();
    GList *ptr = servers;
    while (ptr) {
        JaServerConfig *server = (JaServerConfig *) ptr->data;
        g_printf("%u,%s\n", server->listen_port, server->name);
        ptr = g_list_next(ptr);
    }

    return (0);
}
