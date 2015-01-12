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
#include "jio/jio.h"

int main(int argc, const char *argv[])
{
    JaConfig *jcfg = ja_config_load();
    if (jcfg == NULL) {
        g_printf("fail to parse configuration file\n");
        return -1;
    }

    GList *group = jcfg->groups;
    while (group) {
        JaDirectiveGroup *g = (JaDirectiveGroup *) group->data;
        g_printf("Group:%s\n", g->name);
        GList *keys = g_hash_table_get_keys(g->directives);
        GList *ptr = keys;
        while (ptr) {
            JaDirective *jd =
                (JaDirective *) g_hash_table_lookup(g->directives,
                                                    ptr->data);
            g_printf("\t%s:%s\n", jd->name, jd->args);
            ptr = g_list_next(ptr);
        }
        g_list_free(keys);
        group = g_list_next(group);
    }

    ja_config_free(jcfg);

    // JSocket *jsock = j_server_socket_new(2345, 512);
    // JSocket *csock = NULL;
    // while ((csock = j_socket_accept(jsock)) != NULL) {
    //     j_socket_close(csock);
    // }
    // j_socket_close(jsock);
    return (0);
}
