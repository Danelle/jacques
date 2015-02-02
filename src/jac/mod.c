/*
 * mod.c
 *
 * Copyright (C) 2015 - Wiky L <wiiiky@yeah.net>
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
#include "mod.h"
#include <gmodule.h>


static GList *loaded_modules = NULL;


/* List of all different hooks */
static GList *request_hooks = NULL;
static GList *server_quit_hooks = NULL;


typedef void (*ModuleInitFunc) ();

GList *ja_get_modules()
{
    return loaded_modules;
}


GList *ja_get_request_hooks(void)
{
    return request_hooks;
}


GList *ja_get_server_quit_hooks(void)
{
    return server_quit_hooks;
}


void ja_module_register(JaModule * mod)
{
    loaded_modules = g_list_append(loaded_modules, mod);

    mod->init_func();
}

void ja_hook_register(void *ptr, JaHookType type)
{
    switch (type) {
    case JA_HOOK_TYPE_REQUEST:
        request_hooks = g_list_append(request_hooks, ptr);
        break;
    case JA_HOOK_TYPE_SERVER_QUIT:
        server_quit_hooks = g_list_append(server_quit_hooks, ptr);
        break;
    }
}
