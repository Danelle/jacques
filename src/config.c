/*
 * config.c
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
#include "mod.h"
#include <stdlib.h>

/*
 * Loads modules based on configuration
 */
void ja_config_load_modules(JConfig * cfg)
{
    JDirectiveGroup *global = j_config_lookup(cfg, NULL);

    GList *ptr = global->directives;
    while (ptr) {
        JDirective *jd = (JDirective *) ptr->data;
        if (g_strcmp0(jd->name, DIRECTIVE_LOADMODULE) == 0) {
            ja_load_module(jd->value, cfg);
        }
        ptr = g_list_next(ptr);
    }
}
