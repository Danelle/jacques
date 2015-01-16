/*
 * jconf.c
 *
 * Copyright (C) 2015 - Wiky L
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
#include "jconf.h"
#include "../io/jio.h"


JDirective *j_directive_new(const gchar * name, const gchar * value,
                            const gchar * filename, guint line)
{
    JDirective *jd = (JDirective *) g_slice_alloc(sizeof(JDirective));
    jd->name = g_strdup(name);
    jd->value = g_strdup(value);
    jd->filename = g_strdup(filename);
    jd->line = line;
    return jd;
}

void j_directive_free(JDirective * jd)
{
    g_free(jd->name);
    g_free(jd->value);
    g_free(jd->filename);
    g_slice_free1(sizeof(JDirective), jd);
}


static void j_conf_parse_internal(const gchar * filepath, GHashTable * tb)
{
}

/*
 * Returns a GHashTable, which has conf group as key, a list of JDirective as value
 */
GHashTable *j_conf_parse(const gchar * filepath)
{
}
