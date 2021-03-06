/*
 * config.c
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

#include "config.h"
#include <stdlib.h>
#include <unistd.h>


/*
 * Parses configuration file CONFIG_FILEPATH
 * Returns NULL on error
 */
JaConfig *ja_config_load(GError ** error)
{
    chdir(CONFIG_LOCATION);
    JaConfig *p = j_parse(CONFIG_FILEPATH, error);
    chdir("/");
    return p;
}


static gchar *get_module_name(const gchar * name);

/*
 * Loads modules based on configuration
 */
void ja_config_load_modules(JaConfig * cfg)
{
    GList *ptr = j_parser_get_root(cfg);
    while (ptr) {
        JNode *n = (JNode *) ptr->data;
        if (j_node_is_directive(n)) {
            JDirective *d = j_node_get_directive(n);
            if (g_strcmp0(DIRECTIVE_LOADMODULE,
                          j_directive_get_name(d)) == 0) {
                ja_load_module(j_directive_get_value(d));
            }
        }
        ptr = g_list_next(ptr);
    }

    /* register hooks */
    ptr = ja_get_modules();
    while (ptr) {
        JaModule *mod = (JaModule *) ptr->data;
        JaModuleHooksInit hook_init = mod->hooks_init_func;
        if (hook_init) {
            hook_init();
        }
        ptr = g_list_next(ptr);
    }
}


gboolean ja_load_module(const gchar * name)
{
    gchar *mname = get_module_name(name);
    if (mname == NULL) {
        g_warning("invalid module name %s", name);
        return FALSE;
    }
    gchar path[1024];
    g_snprintf(path, sizeof(path), "%s/%s", CONFIG_MOD_ENABLED_LOCATION,
               name);
    GModule *mod =
        g_module_open(path, G_MODULE_BIND_LOCAL | G_MODULE_BIND_LAZY);
    if (mod == NULL) {
        g_warning("fail to load module %s", name);
        g_free(mname);
        return FALSE;
    }
    g_module_make_resident(mod);

    gchar sym_name[1024];
    g_snprintf(sym_name, sizeof(sym_name), "%s_struct", mname);

    gpointer symbol;
    gint ret = g_module_symbol(mod, sym_name, &symbol);
    if (ret) {
        ja_module_register((JaModule *) symbol);
    } else {
        g_warning("fail to load module %s", name);
    }
    g_module_close(mod);
    g_free(mname);

    return ret ? TRUE : FALSE;
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
