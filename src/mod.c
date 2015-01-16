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
#include "config.h"
#include <gmodule.h>


static GList *loaded_modules = NULL;

static GList *all_hooks = NULL;

static gchar *get_module_name(const gchar * name);


typedef void (*ModuleInitFunc) ();

GList *ja_get_modules()
{
    return loaded_modules;
}

GList *ja_get_hooks()
{
    return all_hooks;
}

void ja_module_register(JaModule * mod)
{
    loaded_modules = g_list_append(loaded_modules, mod);
    mod->init_func();
}

void ja_hook_register(void *ptr, JaHookType type)
{
    all_hooks = g_list_append(all_hooks, ja_hook_new(ptr, type));
}



int ja_load_module(const gchar * name)
{
    gchar *mname = get_module_name(name);
    if (mname == NULL) {
        g_warning("invalid module name %s", name);
        return 0;
    }
    gchar path[1024];
    g_snprintf(path, sizeof(path), "%s/%s", CONFIG_MOD_ENABLED_LOCATION,
               name);
    GModule *mod =
        g_module_open(path, G_MODULE_BIND_LOCAL | G_MODULE_BIND_LAZY);
    if (mod == NULL) {
        g_free(mname);
        return 0;
    }
    g_module_make_resident(mod);

    gchar sym_name[1024];
    g_snprintf(sym_name, sizeof(sym_name), "%s_struct", mname);

    gpointer symbol;
    gint ret = g_module_symbol(mod, sym_name, &symbol);
    if (ret) {
        ja_module_register((JaModule *) symbol);
    }
    g_module_close(mod);
    g_free(mname);

    return ret;
}

/*
 * Loads all modules under CONFIG_MOD_ENABLED_LOCATION
 */
void ja_load_all_modules()
{
    GDir *dir = g_dir_open(CONFIG_MOD_ENABLED_LOCATION, 0, NULL);
    if (!dir) {
        g_warning("fail to open modules directory: %s",
                  CONFIG_MOD_ENABLED_LOCATION);
        return;
    }

    const gchar *name;
    while ((name = g_dir_read_name(dir))) {
        if (!ja_load_module(name)) {
            g_warning("fail to load %s", name);
        }
    }
    g_dir_close(dir);
}

static gchar *get_module_name(const gchar * name)
{
    gint dot = -1, sp = -1;
    gint i = 0;
    while (name[i] != '\0') {
        if (name[i] == '/') {
            sp = i;
        } else if (name[i] == '.') {
            dot = i;
        }
        i++;
    }
    if (dot == -1 && sp == -1) {
        return g_strdup(name);
    } else if (dot > -1 && sp == -1) {
        return g_strndup(name, dot);
    } else if (dot > -1 && sp > -1 && dot > sp) {
        return g_strndup(name + sp + 1, dot - sp - 1);
    } else if (dot == -1 && sp > 0) {
        return g_strdup(name + sp + 1);
    }
    return NULL;
}
