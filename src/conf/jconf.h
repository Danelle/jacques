/*
 * jconf.h
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
#ifndef __J_CONF_H__
#define __J_CONF_H__

#include <glib.h>


typedef struct {
    gchar *name;
    gchar *value;

    gchar *filename;
    guint line;                 /* line number in file */
} JDirective;


JDirective *j_directive_new(const gchar * name, const gchar * value,
                            const gchar * filename, guint line);
void j_directive_free(JDirective * jd);


#define GLOBAL_CONFIG_GROUP   "global"


/*
 * Returns a GHashTable, which has conf group as key, a list of JDirective as value
 */
GHashTable *j_conf_parse(const gchar * filepath);




#endif
