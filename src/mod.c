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


typedef void (*ModuleInitFunc) ();

GList *ja_get_modules()
{
    return loaded_modules;
}

void ja_module_register(JaModule * mod)
{
    loaded_modules = g_list_append(loaded_modules, mod);
}

int ja_load_module(const gchar * name)
{
    gchar path[1024];
    g_snprintf(path, sizeof(path), "%s/%s", CONFIG_MOD_ENABLED_LOCATION,
               name);
    GModule *mod =
        g_module_open(path, G_MODULE_BIND_LOCAL | G_MODULE_BIND_LAZY);
    if (mod == NULL) {
        g_warning("%s", path);
        return 0;
    }
    g_module_make_resident(mod);
    gpointer symbol;
    gint ret = g_module_symbol(mod, JA_MODULE_INIT_NAME, &symbol);
    if (ret) {
        ((ModuleInitFunc) symbol) ();
    }
    g_module_close(mod);

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
